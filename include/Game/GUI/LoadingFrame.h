#ifndef GAME_GUI_LOADINGFRAME_H__
#define GAME_GUI_LOADINGFRAME_H__

#include "GUI/TextureFrame.h"
#include "GUI/Slider.h"
#include "Core/ResourceManager.h"
#include "Game/GUI/ELoadingFrameComplete.h"

namespace dd
{
namespace GUI
{

class LoadingFrame : public TextureFrame
{
public:
	LoadingFrame(Frame* parent, std::string name)
		: TextureFrame(parent, name)
	{
		m_Slider = new GUI::Slider(this, "LoadingFrameSlider");
		//m_SFXSlider->X = 0;
		m_Slider->Y = 453;
		//m_SFXSlider->SetTop(m_TitleSFX->Bottom() + 10);
		m_Slider->SetTexture("Textures/GUI/Menu/SliderBackground.png");
		m_Slider->SetTextureReleased("Textures/GUI/Menu/SliderHandle.png");
		m_Slider->AlignVertically();
		m_Slider->SetPercentage(1.f);
		m_Slider->X = Width / 2.f - m_Slider->Width / 2.f;
	}

	void AddResource(std::string resourceType, std::string resourceName)
	{
		m_Preload.push_back(std::make_pair(resourceType, resourceName));
	}

	void Preload()
	{
		m_Preloading = true;
	}

	void CancelPreload()
	{
		m_Preloading = false;
		m_CurrentItem = 0;
	}

	void Update(double dt) override
	{
		if (m_Preloading) {
			ResourceManager::Preload(m_Preload[m_CurrentItem].first, m_Preload[m_CurrentItem].second);
			m_Slider->SetPercentage((m_CurrentItem + 1) / (float)m_Preload.size());
			m_CurrentItem++;
			if (m_CurrentItem >= m_Preload.size()) {
				m_Preloading = false;
				dd::Events::LoadingFrameComplete e;
				e.FrameName = Name();
				e.Frame = this;
				EventBroker->Publish(e); 
			}
		}
	}

private:
	bool m_Preloading = false;
	int m_CurrentItem = 0;
	std::vector<std::pair<std::string, std::string>> m_Preload;
	GUI::Slider* m_Slider = nullptr;
};

}
}

#endif