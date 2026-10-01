/**
 * @file Application.h
 * @brief アプリケーション全体を管理するクラス
 * @details NewGOを使わずに、アプリ側のオブジェクトの生成・更新・描画・破棄を自分たちで行う。
 *          NewGOを使うのは、k2EngineLowのサウンド再生とエフェクト再生のみ。
 */
#pragma once


namespace app
{
	/** 前方宣言 */
	class Game;


	/**
	 * @brief アプリケーション全体を管理するクラス
	 */
	class Application : public Noncopyable
	{
	public:
		Application();
		~Application();


	public:
		/**
		 * @brief エンジンの更新(g_engine->ExecuteUpdate())の前に行う更新
		 * @details SoundManager / EffectManager が保持しているSoundSource・EffectEmitterの回収を行う。
		 *          これらは再生が終わると自分で DeleteGO して、実際の delete は次の ExecuteUpdate の冒頭で行われる。
		 *          ExecuteUpdate の後に回収すると、解放済みのポインタに触れてしまうため、必ずその前に呼ぶ。
		 */
		void PreUpdate();


		/**
		 * @brief エンジンの更新(g_engine->ExecuteUpdate())の後に行う、アプリ側の更新
		 * @details パッドの入力が更新された後に呼ばれるので、入力は最新の状態を読める。
		 */
		void Update();


		/**
		 * @brief アプリ側の描画
		 * @param rc レンダーコンテキスト
		 */
		void Render(RenderContext& rc);


	private:
		/** ゲーム */
		std::unique_ptr<Game> m_game;
	};
}
