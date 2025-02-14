#include "PS5Window.h"
#include "../TeamProject/TutorialGame.h"
#include "../CSC8503CoreClasses/GameWorld.h"
#include "../TeamProject/GameTechRendererInterface.h"
#include "GameTechAGCRenderer.h"
#include "Debug.h"

using namespace NCL;
using namespace PS5;

size_t sceUserMainThreadStackSize = 2 * 1024 * 1024;
extern const char sceUserMainThreadName[] = "TeamProjectGameMain";
int sceUserMainThreadPriority = SCE_KERNEL_PRIO_FIFO_DEFAULT;
size_t sceLibcHeapSize = 256 * 1024 * 1024;

int main() {
	std::unique_ptr<PS5Window>		window	= std::make_unique<PS5Window>("Hello!", 1920, 1080);
	std::unique_ptr<CSC8503::GameWorld>		world	= std::make_unique<CSC8503::GameWorld>();

#ifdef USEAGC
	std::unique_ptr<CSC8503::GameTechAGCRenderer> renderer = std::make_unique<CSC8503::GameTechAGCRenderer>(*world);

	PS5Controller* c = window->GetController();
#else
	GameTechRenderer* renderer = new GameTechRenderer(*world);
#endif
	std::unique_ptr<CSC8503::TutorialGame> g = std::make_unique<CSC8503::TutorialGame>(*world, *renderer);

	while (window->UpdateWindow()) {
		float dt = window->GetTimer().GetTimeDeltaSeconds();
		g->UpdateGame(dt);
		renderer->Update(dt);
		renderer->Render();
		Debug::UpdateRenderables(dt);
	}

	return 0;
}
