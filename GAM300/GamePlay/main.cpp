#include "Engine.h"

#ifdef  EDITOR
#include "../Editor/src/Editor.h"
#endif


#undef main
int main()
{
	SystemConfig cfg;

	cfg.Load();

	int check = GSM.SystemInit(cfg);

	assert(check == 0);

	GSM.GameLoop();

	GSM.SystemShutDown();

	cfg.Save();

	return 0;

}
