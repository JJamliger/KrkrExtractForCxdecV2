#include "Application.h"
#include "path.h"
#include "util.h"
#include "ExtendUtils.h"

namespace Engine
{
    /// <summary>
    /// ��ʵ��
    /// </summary>
    static Application* g_Instance = nullptr;

    //Hook�������
    tTVPV2LinkProc g_V2Link = nullptr;
    HRESULT __stdcall HookV2Link(iTVPFunctionExporter* exporter)
    {
        HRESULT result = g_V2Link(exporter);
        HookUtils::InlineHook::UnHook(g_V2Link, HookV2Link);
        g_V2Link = nullptr;

        //��ʼ�����
        Application::GetInstance()->InitializeTVPEngine(exporter);

        return result;
    }

    //Hook�������
    auto g_GetProcAddressFunction = GetProcAddress;
    FARPROC WINAPI HookGetProcAddress(HMODULE hModule, LPCSTR lpProcName)
    {
        FARPROC result = g_GetProcAddressFunction(hModule, lpProcName);
        if (result)
        {
            // ������ŵ���
            if (HIWORD(lpProcName) != 0)
            {
                if (strcmp(lpProcName, "V2Link") == 0)
                {
                    //Ntͷƫ��
                    PIMAGE_NT_HEADERS ntHeader = PIMAGE_NT_HEADERS((ULONG_PTR)hModule + ((PIMAGE_DOS_HEADER)hModule)->e_lfanew);
                    //��ѡͷ��С
                    DWORD optionalHeaderSize = ntHeader->FileHeader.SizeOfOptionalHeader;
                    //��һ���ڱ�(�����)
                    PIMAGE_SECTION_HEADER codeSectionHeader = (PIMAGE_SECTION_HEADER)((ULONG_PTR)ntHeader + sizeof(ntHeader->Signature) + sizeof(IMAGE_FILE_HEADER) + optionalHeaderSize);

                    DWORD codeStartRva = codeSectionHeader->VirtualAddress;  //�������ʼRVA
                    DWORD codeSize = codeSectionHeader->SizeOfRawData;		//����δ�С

                    ULONG_PTR codeStartVa = (ULONG_PTR)hModule + codeStartRva;      //�������ʼVA

                    //��ʼ��
                    Application* app = Application::GetInstance();
                    if (!app->IsTVPEngineInitialize())
                    {
                        g_V2Link = (tTVPV2LinkProc)result;
                        HookUtils::InlineHook::Hook(g_V2Link, HookV2Link);
                    }

                    //����ӿ�
                    ExtractCore* extractor = app->GetExtractor();
                    if (!extractor->IsInitialized())
                    {
                        extractor->Initialize((PVOID)codeStartVa, codeSize);
                    }

                    //��ʼ����� ���Hook
                    if (extractor->IsInitialized())
                    {
                        HookUtils::InlineHook::UnHook(g_GetProcAddressFunction, HookGetProcAddress);
                    }
                }
            }
        }
        return result;
    }


    //**********Application***********//
    Application::Application()
    {
        this->mModuleBase = nullptr;

        this->mCurrentDirectoryPath = Path::GetDirectoryName(Util::GetModulePathW(GetModuleHandleW(NULL)));
        this->mTVPExporterInitialized = false;
        this->mExtractor = new ExtractCore();

        std::wstring extractDirectpry = this->mCurrentDirectoryPath + L"\\Archive_Output";
        this->mExtractor->SetOutputDirectory(extractDirectpry);
    }

    Application::~Application()
    {
        if (this->mExtractor)
        {
            delete this->mExtractor;
            this->mExtractor = nullptr;
        }
    }

    void Application::InitializeModule(HMODULE hModule)
    {
        this->mModuleBase = hModule;
        this->mDllDirectoryPath = Path::GetDirectoryName(Util::GetModulePathW(hModule));

        std::wstring extractLogPath = this->mDllDirectoryPath + L"\\ExtractCore.log";
        this->mExtractor->SetLoggerPath(extractLogPath);
    }

    void Application::InitializeTVPEngine(iTVPFunctionExporter* exporter)
    {
        this->mTVPExporterInitialized = TVPInitImportStub(exporter);
        TVPSetCommandLine(L"-debugwin", L"yes");
    }

    bool Application::IsTVPEngineInitialize()
    {
        return this->mTVPExporterInitialized;
    }

    ExtractCore* Application::GetExtractor()
    {
        return this->mExtractor;
    }

    //====Static====//

    Application* Application::GetInstance()
    {
        return g_Instance;
    }

    void Application::Initialize(HMODULE hModule)
    {
        g_Instance = new Application();
        g_Instance->InitializeModule(hModule);
        HookUtils::InlineHook::Hook(g_GetProcAddressFunction, HookGetProcAddress);
    }

    void Application::Release()
    {
        if (g_Instance)
        {
            delete g_Instance;
        }
        g_Instance = nullptr;
    }

    //================================//
}

