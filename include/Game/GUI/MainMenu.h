#ifndef GUI_MAINMENU_H__
#define GUI_MAINMENU_H__

#include "GUI/TextureFrame.h"
#include "Game/GUI/LoadingFrame.h"
#include "Game/GUI/MainMenuMain.h"
#include "Game/GUI/MainMenuOptions.h"
#include "Core/EKeyUp.h"
#include "Game/EGameStart.h"

namespace dd
{
namespace GUI
{

class MainMenu : public TextureFrame
{
public:
	MainMenu(Frame* parent, std::string name)
		: TextureFrame(parent, name)
	{
		SetTexture("Textures/GUI/Menu/Background.png");
		Width = 675;
		Height = 1080;

		m_LeftBanner = new GUI::TextureFrame(this, "MainMenuLeftBanner");
		m_LeftBanner->SetTexture("Textures/GUI/Menu/Left.png");
		m_LeftBanner->Width = 231;
		m_LeftBanner->Height = 1080;

		m_MainMenuMain = new GUI::MainMenuMain(this, "MainMenuMain");
		m_MainMenuMain->SetLeft(m_LeftBanner->Right());
		m_MainMenuMain->SetTop(60);

		m_MainMenuOptions = new GUI::MainMenuOptions(this, "MainMenuOptions");
		m_MainMenuOptions->SetLeft(m_LeftBanner->Right());
		m_MainMenuOptions->SetTop(m_MainMenuMain->Bottom());

		//EVENT_SUBSCRIBE_MEMBER(m_EKeyUp, &MainMenu::OnKeyUp);
	}

	void Show() override
	{
		if (ResourceManager::Load<ConfigFile>("Config.ini")->GetValue<bool>("Debug.SkipMenu", false)) {
			Events::GameStart e;
			EventBroker->Publish(e);
			Hide();
			auto hud = new GUI::HUD(BaseFrame, "HUD");
		} else {
			TextureFrame::Show();
		}
	}

private:
	TextureFrame* m_LeftBanner;
	MainMenuMain* m_MainMenuMain;
	MainMenuOptions* m_MainMenuOptions;

	//EventRelay<Frame, Events::KeyUp> m_EKeyUp;
	bool OnKeyUp(const Events::KeyUp& event)
	{
		if (event.KeyCode == GLFW_KEY_ESCAPE) {
			Show();
		}

		return true;
	}
};

}
}
#endif

