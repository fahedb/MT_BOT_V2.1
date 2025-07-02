#include "MainForm.h"
#include "Settings.h"

System::Void MTAnalyzer::MainForm::InitRadioFilterSelection(PatternsCompare_FilterType type)
{
	if (type == PatternsCompare_FilterType::ALL)
	{
		om_AllSelectionRadioButton->Checked = true;
		om_TradesOnlyRadioButton->Checked = false;
		om_PatternLabelName->Text = L"Scan Time:";
	}
	else if (type == PatternsCompare_FilterType::TradesOnly)
	{
		om_AllSelectionRadioButton->Checked = false;
		om_TradesOnlyRadioButton->Checked = true;
		om_PatternLabelName->Text = L"Trade SeqNum:";
	}
	else // Defualt case is to choose by Trade seqNum
	{
		om_AllSelectionRadioButton->Checked = false;
		om_TradesOnlyRadioButton->Checked = true;
		om_PatternLabelName->Text = L"Trade SeqNum:";

	}
}
// Function to init the relevant patterns filter of the choose combo box

System::Void MTAnalyzer::MainForm::InitGraphType(PatternCompare_GraphType type)
{
	if (type == PatternCompare_GraphType::Bars)
		Settings::getInstance()->Get_Policy()->GraphType = PatternCompare_GraphType::Bars;
	else if (type == PatternCompare_GraphType::Linear)
		Settings::getInstance()->Get_Policy()->GraphType = PatternCompare_GraphType::Linear;

	// TODO: Need to add a kind of reload function to reload the patterns
}
