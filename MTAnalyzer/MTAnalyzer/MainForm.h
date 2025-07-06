#include "Settings.h"
#include "ProxyGraphStruct.h"
#include "GraphsManager.h"
//#include "GraphContainerCache.h"

#pragma once

/// <summary>
/// cm = const member
/// om = object member
/// m = member
/// </summary>

enum class ButtonDirection
{
	Prev,
	Next
};

namespace MTAnalyzer {
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Collections::Generic;
	/// <summary>
	/// Summary for MainForm
	/// </summary>
	public ref class MainForm : public System::Windows::Forms::Form
	{
	public:
		MainForm(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~MainForm()
		{
			if (components)
			{
				delete components;
			}
		}

	private: System::Windows::Forms::PictureBox^ om_CurrPatternPicContainer;
	private: System::Windows::Forms::PictureBox^ om_HistoryPatternPicContainer;
	private: System::Windows::Forms::ComboBox^ om_PatternComboBoxSelection;
	private: System::Windows::Forms::Label^ om_PatternLabelName;
	private: System::Windows::Forms::GroupBox^ om_PatternsGroupSelection;
	private: System::Windows::Forms::RadioButton^ om_TradesOnlyRadioButton;
	private: System::Windows::Forms::Button^ om_NextPatternButt;
	private: System::Windows::Forms::Button^ om_PrevPatternButt;
	private: System::Windows::Forms::Button^ om_MoveToCombinePatternForm;
	private: System::Windows::Forms::RadioButton^ om_AllSelectionRadioButton;

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container^ components;
	private:
		const String^ cm_AllSelectionStr = L"ALL";
		const String^ cm_TradesOnlySelectionStr = L"Trades Only";

	private: System::Windows::Forms::GroupBox^ om_GraphTypeSelection;
	private: System::Windows::Forms::RadioButton^ om_BarsGraphRadioButt;
	private: System::Windows::Forms::RadioButton^ om_LinearGraphRadioButt;
	private: System::Windows::Forms::Button^ om_RefreshPatternsObjectsButt;

#pragma region Windows Form Designer generated code
		   /// <summary>
		   /// Required method for Designer support - do not modify
		   /// the contents of this method with the code editor.
		   /// </summary>
		   void InitializeComponent(void);
#pragma endregion
	private:
		//GraphContainerCache* m_graphsCache;
		List < Tuple<ProxyGraphStruct^, List<ProxyGraphStruct^>^>^>^ m_allGraphsList; // List of all graphs

		int m_currentPatternIndex; // Current current pattern index in the list
		int m_historyPatternIndex; // Current history pattern index in the list
	private:
		System::Void AllSelectionRadioButton_CheckedChanged(System::Object^ sender, System::EventArgs^ e) { if (om_AllSelectionRadioButton->Checked == true) InitRadioFilterSelection(PatternsCompare_FilterType::ALL); }
		System::Void TradesOnlyRadioButton_CheckedChanged(System::Object^ sender, System::EventArgs^ e) { if (om_TradesOnlyRadioButton->Checked == true) InitRadioFilterSelection(PatternsCompare_FilterType::TradesOnly); }
		System::Void BarsGraphRadioButt_CheckedChanged(System::Object^ sender, System::EventArgs^ e) { if (om_BarsGraphRadioButt->Checked == true) InitGraphType(PatternCompare_GraphType::Bars); }
		System::Void LinearGraphRadioButt_CheckedChanged(System::Object^ sender, System::EventArgs^ e) { if (om_LinearGraphRadioButt->Checked == true) InitGraphType(PatternCompare_GraphType::Linear); }

		System::Void PrevHistoryPatternButt_Click(System::Object^ sender, System::EventArgs^ e)
		{
			InitHistoryImageBox(om_HistoryPatternPicContainer, ButtonDirection::Prev); // Initialize the history image box with the previous pattern
		}
		System::Void NextHistoryPatternButt_Click(System::Object^ sender, System::EventArgs^ e)
		{
			InitHistoryImageBox(om_HistoryPatternPicContainer, ButtonDirection::Next); // Initialize the history image box with the next pattern
		}

	private:
		System::Void InitRadioFilterSelection(PatternsCompare_FilterType type); // Function to init the relevant filter of the choose combo box
		System::Void InitGraphType(PatternCompare_GraphType type); // Function to init graph type
		System::Void InitGraphsList(); // Function to initialize the graphs list
		System::Void ResetGraphsList(); // Function to reset the graphs list
		System::Void InitHistoryImageBox(System::Windows::Forms::PictureBox^ imageBox, ButtonDirection direction);

	private: System::Void om_RefreshPatternsObjectsButt_Click(System::Object^ sender, System::EventArgs^ e) {
		ResetGraphsList(); // Reset the graphs list
		InitGraphsList(); // Initialize the graphs list
	}

	private: System::Void om_PatternComboBoxSelection_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e);

	};
};
