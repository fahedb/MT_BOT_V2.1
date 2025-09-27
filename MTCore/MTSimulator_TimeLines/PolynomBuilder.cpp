#include "PolynomBuilder.h"
#include "LOGGER.H"


Polynom PolynomBuilder::BuildAPolynom(GraphStructure& graph)
{
    auto polyVec = BuildPolynomialVector(graph.points);
    Polynom polynom;
    polynom.SetPolynom(polyVec);
    return polynom;
}

// Builds a polynomial using the Vandermonde matrix method
std::vector<double> PolynomBuilder::BuildPolynomialVector(std::vector<std::pair<double, double>>& points) {
    const int n = points.size();
    std::vector<std::vector<double>> A(n, std::vector<double>(n));
    std::vector<double> b(n);

    // Build the Vandermonde matrix A and the vector b
    for (int i = 0; i < n; ++i) {
        double xi = points[i].first;
        double val = 1.0;
        for (int j = 0; j < n; ++j) {
            A[i][j] = val;
            val *= xi;
        }
        b[i] = points[i].second;
    }

    std::vector<double> coeffs;
    if (!SolveLinearSystem(A, b, coeffs)) {
        Logger::getInstance().WriteLog("Failed to solve the linear system for polynomial coefficients.", true, true, LogType::ERR);
        return {};
    }

    return coeffs; // Polynomial coefficients in descending order
}

bool PolynomBuilder::SolveLinearSystem(std::vector<std::vector<double>>& A, std::vector<double>& b, std::vector<double>& x) {
    const int n = A.size();
    x.resize(n);

    for (int i = 0; i < n; ++i) {
        // looking for the pivot row
        int pivot = i;
        for (int j = i + 1; j < n; ++j)
            if (std::fabs(A[j][i]) > std::fabs(A[pivot][i]))
                pivot = j;

        if (std::fabs(A[pivot][i]) < 1e-12)
            return false; // unsolvable system (singular matrix)

        std::swap(A[i], A[pivot]);
        std::swap(b[i], b[pivot]);

        // normalization for the pivot row
        for (int j = i + 1; j < n; ++j) {
            double factor = A[j][i] / A[i][i];
            for (int k = i; k < n; ++k)
                A[j][k] -= factor * A[i][k];
            b[j] -= factor * b[i];
        }
    }

    // back substitution
    for (int i = n - 1; i >= 0; --i) {
        x[i] = b[i];
        for (int j = i + 1; j < n; ++j)
            x[i] -= A[i][j] * x[j];
        x[i] /= A[i][i];
    }

    return true;
}
