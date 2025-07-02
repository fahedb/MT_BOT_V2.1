#include <cstdlib>
#include "MainForm.h"

using namespace System;
using namespace System::Windows::Forms;

int main()
{
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);

	MTAnalyzer::MainForm mainF;

	System::Windows::Forms::Application::Run(% mainF);

	return 0;
}