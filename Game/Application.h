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
	class ParamDebugUI;
	class SoundDebugUI;


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
		 * @details SoundManagerとEffectManagerが保持している、再生が終わった音・エフェクトの回収を行う。
		 *          再生が終わった音・エフェクトはエンジンが自分でDeleteGOし、実際のdeleteは次のExecuteUpdateの冒頭で行われる。
		 *          ExecuteUpdateの後に回収すると解放済みのポインタに触れうるので、必ずその前に呼ぶ。
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


		/**
		 * @brief アプリ側のUIの描画
		 * @details RenderingEngine::Execute()の後に呼ばれる。
		 *          Execute()の中のポストプロセスがフレームバッファーを全面コピーで上書きするので、
		 *          Renderで描いたスプライトは見えなくなる。ポストプロセスの後に即時描画したいもの(UI・フェードなど)はここで描く。
		 *          後から描いたものが手前に出るので、最前面にしたいものほど後に描くこと。
		 * @param rc レンダーコンテキスト
		 */
		void RenderUI(RenderContext& rc);


	private:
		/** ゲーム */
		std::unique_ptr<Game> m_game;

		/** サウンドの音量調整用UI */
		std::unique_ptr<SoundDebugUI> m_soundDebugUI;

		/** パラメーターの状態確認用UI */
		std::unique_ptr<ParamDebugUI> m_paramDebugUI;
	};
}
