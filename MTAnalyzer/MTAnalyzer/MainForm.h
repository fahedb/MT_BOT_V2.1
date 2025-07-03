#include "Settings.h"
//#include "GraphContainerCache.h"

#pragma once

/// <summary>
/// cm = const member
/// om = object member
/// m = member
/// </summary>

namespace MTAnalyzer {


	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

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
	protected:



	protected:


	private: System::Windows::Forms::ComboBox^ om_PatternComboBoxSelection;

	private: System::Windows::Forms::Label^ om_PatternLabelName;
	private: System::Windows::Forms::GroupBox^ om_PatternsGroupSelection;
	private: System::Windows::Forms::RadioButton^ om_TradesOnlyRadioButton;
	private: System::Windows::Forms::Button^ om_NextPatternButt;








	private: System::Windows::Forms::Button^ om_PrevPatternButt;
	private: System::Windows::Forms::Button^ om_MoveToCombinePatternForm;







	private: System::Windows::Forms::RadioButton^ om_AllSelectionRadioButton;




	protected:


	protected:

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;
	private:
		const String^ cm_AllSelectionStr = L"ALL";
	private: System::Windows::Forms::GroupBox^ om_GraphTypeSelection;
	private: System::Windows::Forms::RadioButton^ om_BarsGraphRadioButt;
	private: System::Windows::Forms::RadioButton^ om_LinearGraphRadioButt;
	private: System::Windows::Forms::Button^ om_RefreshPatternsObjectsButt;

		   const String^ cm_TradesOnlySelectionStr = L"Trades Only";

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->om_CurrPatternPicContainer = (gcnew System::Windows::Forms::PictureBox());
			this->om_HistoryPatternPicContainer = (gcnew System::Windows::Forms::PictureBox());
			this->om_PatternComboBoxSelection = (gcnew System::Windows::Forms::ComboBox());
			this->om_PatternLabelName = (gcnew System::Windows::Forms::Label());
			this->om_PatternsGroupSelection = (gcnew System::Windows::Forms::GroupBox());
			this->om_TradesOnlyRadioButton = (gcnew System::Windows::Forms::RadioButton());
			this->om_AllSelectionRadioButton = (gcnew System::Windows::Forms::RadioButton());
			this->om_NextPatternButt = (gcnew System::Windows::Forms::Button());
			this->om_PrevPatternButt = (gcnew System::Windows::Forms::Button());
			this->om_MoveToCombinePatternForm = (gcnew System::Windows::Forms::Button());
			this->om_GraphTypeSelection = (gcnew System::Windows::Forms::GroupBox());
			this->om_BarsGraphRadioButt = (gcnew System::Windows::Forms::RadioButton());
			this->om_LinearGraphRadioButt = (gcnew System::Windows::Forms::RadioButton());
			this->om_RefreshPatternsObjectsButt = (gcnew System::Windows::Forms::Button());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->om_CurrPatternPicContainer))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->om_HistoryPatternPicContainer))->BeginInit();
			this->om_PatternsGroupSelection->SuspendLayout();
			this->om_GraphTypeSelection->SuspendLayout();
			this->SuspendLayout();
			// 
			// om_CurrPatternPicContainer
			// 
			this->om_CurrPatternPicContainer->Location = System::Drawing::Point(776, 12);
			this->om_CurrPatternPicContainer->Name = L"om_CurrPatternPicContainer";
			this->om_CurrPatternPicContainer->Size = System::Drawing::Size(573, 337);
			this->om_CurrPatternPicContainer->TabIndex = 0;
			this->om_CurrPatternPicContainer->TabStop = false;
			// 
			// om_HistoryPatternPicContainer
			// 
			this->om_HistoryPatternPicContainer->Location = System::Drawing::Point(106, 12);
			this->om_HistoryPatternPicContainer->Name = L"om_HistoryPatternPicContainer";
			this->om_HistoryPatternPicContainer->Size = System::Drawing::Size(573, 337);
			this->om_HistoryPatternPicContainer->TabIndex = 1;
			this->om_HistoryPatternPicContainer->TabStop = false;
			// 
			// om_PatternComboBoxSelection
			// 
			this->om_PatternComboBoxSelection->FormattingEnabled = true;
			this->om_PatternComboBoxSelection->Location = System::Drawing::Point(453, 386);
			this->om_PatternComboBoxSelection->Name = L"om_PatternComboBoxSelection";
			this->om_PatternComboBoxSelection->Size = System::Drawing::Size(147, 21);
			this->om_PatternComboBoxSelection->TabIndex = 2;
			// 
			// om_PatternLabelName
			// 
			this->om_PatternLabelName->AutoSize = true;
			this->om_PatternLabelName->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->om_PatternLabelName->Location = System::Drawing::Point(319, 387);
			this->om_PatternLabelName->Name = L"om_PatternLabelName";
			this->om_PatternLabelName->Size = System::Drawing::Size(133, 20);
			this->om_PatternLabelName->TabIndex = 3;
			this->om_PatternLabelName->Text = L"Trade SeqNum:";
			// 
			// om_PatternsGroupSelection
			// 
			this->om_PatternsGroupSelection->Controls->Add(this->om_TradesOnlyRadioButton);
			this->om_PatternsGroupSelection->Controls->Add(this->om_AllSelectionRadioButton);
			this->om_PatternsGroupSelection->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Bold,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->om_PatternsGroupSelection->Location = System::Drawing::Point(106, 387);
			this->om_PatternsGroupSelection->Name = L"om_PatternsGroupSelection";
			this->om_PatternsGroupSelection->Size = System::Drawing::Size(186, 120);
			this->om_PatternsGroupSelection->TabIndex = 4;
			this->om_PatternsGroupSelection->TabStop = false;
			this->om_PatternsGroupSelection->Text = L"Choose Pattern selection";
			// 
			// om_TradesOnlyRadioButton
			// 
			this->om_TradesOnlyRadioButton->AutoSize = true;
			this->om_TradesOnlyRadioButton->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, System::Drawing::FontStyle::Bold,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->om_TradesOnlyRadioButton->Location = System::Drawing::Point(6, 68);
			this->om_TradesOnlyRadioButton->Name = L"om_TradesOnlyRadioButton";
			this->om_TradesOnlyRadioButton->Size = System::Drawing::Size(117, 22);
			this->om_TradesOnlyRadioButton->TabIndex = 5;
			this->om_TradesOnlyRadioButton->TabStop = true;
			this->om_TradesOnlyRadioButton->Text = L"Trades Only";
			this->om_TradesOnlyRadioButton->UseVisualStyleBackColor = true;
			this->om_TradesOnlyRadioButton->CheckedChanged += gcnew System::EventHandler(this, &MainForm::TradesOnlyRadioButton_CheckedChanged);
			// 
			// om_AllSelectionRadioButton
			// 
			this->om_AllSelectionRadioButton->AutoSize = true;
			this->om_AllSelectionRadioButton->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, System::Drawing::FontStyle::Bold,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->om_AllSelectionRadioButton->Location = System::Drawing::Point(6, 31);
			this->om_AllSelectionRadioButton->Name = L"om_AllSelectionRadioButton";
			this->om_AllSelectionRadioButton->Size = System::Drawing::Size(54, 22);
			this->om_AllSelectionRadioButton->TabIndex = 4;
			this->om_AllSelectionRadioButton->TabStop = true;
			this->om_AllSelectionRadioButton->Text = L"ALL";
			this->om_AllSelectionRadioButton->UseVisualStyleBackColor = true;
			this->om_AllSelectionRadioButton->CheckedChanged += gcnew System::EventHandler(this, &MainForm::AllSelectionRadioButton_CheckedChanged);
			// 
			// om_NextPatternButt
			// 
			this->om_NextPatternButt->Location = System::Drawing::Point(1110, 369);
			this->om_NextPatternButt->Name = L"om_NextPatternButt";
			this->om_NextPatternButt->Size = System::Drawing::Size(150, 53);
			this->om_NextPatternButt->TabIndex = 5;
			this->om_NextPatternButt->Text = L"NEXT >>";
			this->om_NextPatternButt->UseVisualStyleBackColor = true;
			this->om_NextPatternButt->Click += gcnew System::EventHandler(this, &MainForm::NextPatternButt_Click);
			// 
			// om_PrevPatternButt
			// 
			this->om_PrevPatternButt->Location = System::Drawing::Point(922, 369);
			this->om_PrevPatternButt->Name = L"om_PrevPatternButt";
			this->om_PrevPatternButt->Size = System::Drawing::Size(150, 53);
			this->om_PrevPatternButt->TabIndex = 6;
			this->om_PrevPatternButt->Text = L"<< PREV";
			this->om_PrevPatternButt->UseVisualStyleBackColor = true;
			this->om_PrevPatternButt->Click += gcnew System::EventHandler(this, &MainForm::PrevPatternButt_Click);
			// 
			// om_MoveToCombinePatternForm
			// 
			this->om_MoveToCombinePatternForm->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->om_MoveToCombinePatternForm->BackColor = System::Drawing::SystemColors::ActiveCaption;
			this->om_MoveToCombinePatternForm->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, System::Drawing::FontStyle::Bold,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->om_MoveToCombinePatternForm->Location = System::Drawing::Point(1189, 570);
			this->om_MoveToCombinePatternForm->Name = L"om_MoveToCombinePatternForm";
			this->om_MoveToCombinePatternForm->Size = System::Drawing::Size(217, 81);
			this->om_MoveToCombinePatternForm->TabIndex = 7;
			this->om_MoveToCombinePatternForm->Text = L"Combine Patterns";
			this->om_MoveToCombinePatternForm->UseVisualStyleBackColor = false;
			// 
			// om_GraphTypeSelection
			// 
			this->om_GraphTypeSelection->Controls->Add(this->om_BarsGraphRadioButt);
			this->om_GraphTypeSelection->Controls->Add(this->om_LinearGraphRadioButt);
			this->om_GraphTypeSelection->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->om_GraphTypeSelection->Location = System::Drawing::Point(106, 513);
			this->om_GraphTypeSelection->Name = L"om_GraphTypeSelection";
			this->om_GraphTypeSelection->Size = System::Drawing::Size(186, 116);
			this->om_GraphTypeSelection->TabIndex = 8;
			this->om_GraphTypeSelection->TabStop = false;
			this->om_GraphTypeSelection->Text = L"Choose Graph Type";
			// 
			// om_BarsGraphRadioButt
			// 
			this->om_BarsGraphRadioButt->AutoSize = true;
			this->om_BarsGraphRadioButt->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, System::Drawing::FontStyle::Bold,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->om_BarsGraphRadioButt->Location = System::Drawing::Point(6, 71);
			this->om_BarsGraphRadioButt->Name = L"om_BarsGraphRadioButt";
			this->om_BarsGraphRadioButt->Size = System::Drawing::Size(61, 22);
			this->om_BarsGraphRadioButt->TabIndex = 7;
			this->om_BarsGraphRadioButt->TabStop = true;
			this->om_BarsGraphRadioButt->Text = L"Bars";
			this->om_BarsGraphRadioButt->UseVisualStyleBackColor = true;
			this->om_BarsGraphRadioButt->CheckedChanged += gcnew System::EventHandler(this, &MainForm::BarsGraphRadioButt_CheckedChanged);
			// 
			// om_LinearGraphRadioButt
			// 
			this->om_LinearGraphRadioButt->AutoSize = true;
			this->om_LinearGraphRadioButt->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, System::Drawing::FontStyle::Bold,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->om_LinearGraphRadioButt->Location = System::Drawing::Point(6, 34);
			this->om_LinearGraphRadioButt->Name = L"om_LinearGraphRadioButt";
			this->om_LinearGraphRadioButt->Size = System::Drawing::Size(72, 22);
			this->om_LinearGraphRadioButt->TabIndex = 6;
			this->om_LinearGraphRadioButt->TabStop = true;
			this->om_LinearGraphRadioButt->Text = L"Linear";
			this->om_LinearGraphRadioButt->UseVisualStyleBackColor = true;
			this->om_LinearGraphRadioButt->CheckedChanged += gcnew System::EventHandler(this, &MainForm::LinearGraphRadioButt_CheckedChanged);
			// 
			// om_RefreshPatternsObjectsButt
			// 
			this->om_RefreshPatternsObjectsButt->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->om_RefreshPatternsObjectsButt->BackColor = System::Drawing::SystemColors::ControlDark;
			this->om_RefreshPatternsObjectsButt->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, System::Drawing::FontStyle::Bold,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->om_RefreshPatternsObjectsButt->Location = System::Drawing::Point(453, 570);
			this->om_RefreshPatternsObjectsButt->Name = L"om_RefreshPatternsObjectsButt";
			this->om_RefreshPatternsObjectsButt->Size = System::Drawing::Size(147, 59);
			this->om_RefreshPatternsObjectsButt->TabIndex = 9;
			this->om_RefreshPatternsObjectsButt->Text = L"Refresh !";
			this->om_RefreshPatternsObjectsButt->UseVisualStyleBackColor = false;
			// 
			// MainForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(1418, 663);
			this->Controls->Add(this->om_RefreshPatternsObjectsButt);
			this->Controls->Add(this->om_GraphTypeSelection);
			this->Controls->Add(this->om_MoveToCombinePatternForm);
			this->Controls->Add(this->om_PrevPatternButt);
			this->Controls->Add(this->om_NextPatternButt);
			this->Controls->Add(this->om_PatternsGroupSelection);
			this->Controls->Add(this->om_HistoryPatternPicContainer);
			this->Controls->Add(this->om_PatternComboBoxSelection);
			this->Controls->Add(this->om_CurrPatternPicContainer);
			this->Controls->Add(this->om_PatternLabelName);
			this->Name = L"MainForm";
			this->Text = L"MainForm";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->om_CurrPatternPicContainer))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->om_HistoryPatternPicContainer))->EndInit();
			this->om_PatternsGroupSelection->ResumeLayout(false);
			this->om_PatternsGroupSelection->PerformLayout();
			this->om_GraphTypeSelection->ResumeLayout(false);
			this->om_GraphTypeSelection->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
private:
	//GraphContainerCache* m_graphsCache;
private:
	System::Void AllSelectionRadioButton_CheckedChanged(System::Object^ sender, System::EventArgs^ e) { if (om_AllSelectionRadioButton->Checked == true) { InitRadioFilterSelection(PatternsCompare_FilterType::ALL); } }
	System::Void TradesOnlyRadioButton_CheckedChanged(System::Object^ sender, System::EventArgs^ e) { if (om_TradesOnlyRadioButton->Checked == true) InitRadioFilterSelection(PatternsCompare_FilterType::TradesOnly); }
	System::Void BarsGraphRadioButt_CheckedChanged(System::Object^ sender, System::EventArgs^ e) { if (om_BarsGraphRadioButt->Checked == true) InitGraphType(PatternCompare_GraphType::Bars); }
	System::Void LinearGraphRadioButt_CheckedChanged(System::Object^ sender, System::EventArgs^ e) { if (om_LinearGraphRadioButt->Checked == true) InitGraphType(PatternCompare_GraphType::Linear); }
	System::Void PrevPatternButt_Click(System::Object^ sender, System::EventArgs^ e) {}
	System::Void NextPatternButt_Click(System::Object^ sender, System::EventArgs^ e) 
	{
	}

private:
	System::Void InitRadioFilterSelection(PatternsCompare_FilterType type); // Function to init the relevant filter of the choose combo box
	System::Void InitGraphType(PatternCompare_GraphType type); // Function to init graph type

};
}
