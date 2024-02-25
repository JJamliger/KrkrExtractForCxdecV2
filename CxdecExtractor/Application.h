#pragma once

#include <string>
#include "ExtractCore.h"
namespace Engine
{
	using tTVPV2LinkProc = HRESULT(__stdcall*)(iTVPFunctionExporter*);
	using tTVPV2UnlinkProc = HRESULT(__stdcall*)();

	class Application
	{
	private:
		Application();
		Application(const Application&) = delete;
		Application(Application&&) = delete;
		Application& operator=(const Application&) = delete;
		Application& operator=(Application&&) = delete;
		~Application();

	private:
		HMODULE mModuleBase;				//dll������ַ
		std::wstring mDllDirectoryPath;		//dllĿ¼
		std::wstring mCurrentDirectoryPath;	  //��Ϸ��ǰĿ¼
		ExtractCore* mExtractor;			//�����
		bool mTVPExporterInitialized;		//�����ʼ���ɹ���־

	public:

		/// <summary>
		/// ����ģ����Ϣ
		/// </summary>
		/// <param name="hModule">ģ����Ϣ</param>
		void InitializeModule(HMODULE hModule);

		/// <summary>
		/// ��ʼ�����
		/// </summary>
		/// <param name="exporter">�����������</param>
		void InitializeTVPEngine(iTVPFunctionExporter* exporter);

		/// <summary>
		/// ��ȡ����Ƿ��ʼ�����
		/// </summary>
		/// <returns>True�ѳ�ʼ�� Falseδ��ʼ��</returns>
		bool IsTVPEngineInitialize();

		/// <summary>
		/// ��ȡ�����
		/// </summary>
		/// <returns></returns>
		ExtractCore* GetExtractor();

		/// <summary>
		/// ��ȡ����ʵ��
		/// </summary>
		static Application* GetInstance();
		/// <summary>
		/// ��ʼ��
		/// </summary>
		/// <param name="hModule">ģ����Ϣ</param>
		static void Initialize(HMODULE hModule);
		/// <summary>
		/// �ͷ�
		/// </summary>
		static void Release();
	};
}



