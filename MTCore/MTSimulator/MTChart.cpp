#include "MTChart.h"

/// <MTChart>
/// MTChart class function, for drawing charts using x,y points
/// <MTChart>


void MTChart::draw_chart(const GraphStructure& points, const char* filename) {
    const int width = 800, height = 600;
    const int margin = 50;
    int splitIndex = points.splitter;
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t* cr = cairo_create(surface);

    // Background
    cairo_set_source_rgb(cr, 1, 1, 1); // White
    cairo_paint(cr);

    // Draw axes
    cairo_set_source_rgb(cr, 0, 0, 0); // Black
    cairo_set_line_width(cr, 2);
    cairo_move_to(cr, static_cast<double>(margin), static_cast<double>(height - margin)); // X-axis
    cairo_line_to(cr, static_cast<double>(width - margin), static_cast<double>(height - margin));
    cairo_move_to(cr, margin, static_cast<double>(height - margin)); // Y-axis
    cairo_line_to(cr, margin, margin);
    cairo_stroke(cr);

    // Calculate scale
    double x_scale = (static_cast<double>(width) - 2.0 * static_cast<double>(margin)) / (m_Bars);
    double y_scale = (static_cast<double>(height) - 2.0 * static_cast<double>(margin)) / (m_pipsDiff);

    auto transform_x = [&](double x) { return margin + x * x_scale; };
    auto transform_y = [&](double y) { return static_cast<double>(height) - static_cast<double>(margin) - y * y_scale; };

    if (!points.points.empty()) {
        // Decide the split
        // For example, draw the first 8 points in blue and the rest in red

        // Ensure splitIndex is not out of range
        if (splitIndex <= 0 || splitIndex > static_cast<int>(points.points.size())) {
            splitIndex = static_cast<int>(points.points.size());
        }

        // Draw first part in blue (points[0] through points[splitIndex-1])
        cairo_set_source_rgb(cr, 0, 0, 1); // Blue
        cairo_set_line_width(cr, 2);

        cairo_move_to(cr, transform_x(points.points.at(1).first), transform_y(points.points.at(1).second));
        for (size_t i = 1; i < splitIndex; ++i) {
            cairo_line_to(cr, transform_x(points.points.at(i).first), transform_y(points.points.at(i).second));
        }
        cairo_stroke(cr); // Finalize the first segment

        // If there are remaining points, draw them in another color
        if (splitIndex > 0 && splitIndex < points.points.size()) {
            cairo_set_source_rgb(cr, 1, 0, 0); // Red
            cairo_set_line_width(cr, 2);

            // Start from the last point of the previous segment to maintain continuity if you like,
            // or start fresh. Usually, you'd want continuity:
            cairo_move_to(cr, transform_x(points.points.at(static_cast<size_t>(splitIndex) - static_cast<size_t>(1)).first), transform_y(points.points.at(static_cast<size_t>(splitIndex) - static_cast<size_t>(1)).second));

            for (size_t i = splitIndex; i < points.points.size(); ++i) {
                cairo_line_to(cr, transform_x(points.points.at(i).first), transform_y(points.points.at(i).second));
            }
            cairo_stroke(cr);
        }
    }

    // Save to PNG
    cairo_surface_write_to_png(surface, filename);

    // Clean up
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}