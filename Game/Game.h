#pragma once
#include "IObject.h"


namespace app
{
	class Game : public IObject
	{
	public:
		Game() {}
		~Game() {}

	protected:
		void Start() override;
		void Update() override;
		void Render(RenderContext& rc) override;

	private:
		ModelRender m_modelRender;
		ModelRender m_bgModelRender;
		Vector3 m_pos;
	};
}
