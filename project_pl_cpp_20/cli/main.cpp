#include "process.h"

//
int main(int argc, char* argv[])
{
	CExceptionReport::GetInstance().ExceptionHandlerBegin();

	//
	//LoadConfig();

	//
	App app;
	if (!app.Init()) {
		LogError("App Init fail.");
		return 1;
	}

	//
	g_Log.Write(L"system: start.");

	if (!app.Start()) {
		LogError("App Start fail");
		return 1;
	}

	app.Wait();

	//
	g_Log.Write(L"system: end.");
	return 0;
}
