#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include "tp_stub.h"
#include "log.h"

namespace Engine
{
    /// <summary>
    /// �ļ���
    /// </summary>
    class FileEntry
    {
    public:
        /// <summary>
        /// �ļ���Hash
        /// </summary>
        unsigned __int8 DirectoryPathHash[8];
        /// <summary>
        /// �ļ���Hash
        /// </summary>
        unsigned __int8 FileNameHash[32];
        /// <summary>
        /// �ļ�Key
        /// </summary>
        __int64 Key;
        /// <summary>
        /// �ļ����
        /// </summary>
        __int64 Ordinal;

        /// <summary>
        /// ��ȡ�Ϸ���
        /// </summary>
        __declspec(noinline)
        bool IsVaild() const
        {
            return this->Ordinal >= 0;
        }

        /// <summary>
        /// ��ȡ����ģʽ
        /// </summary>
        __declspec(noinline)
        unsigned __int32 GetEncryptMode() const
        {
            return ((this->Ordinal & 0x0000FFFF00000000) >> 32);
        }

        /// <summary>
        /// ��ȡ���������
        /// <para>���8�ֽ� 4���ַ� 3��Unicode�ַ� + 0������</para>
        /// </summary>
        /// <param name="retValue">�ַ�����ֵָ��</param>
        __declspec(noinline)
        void GetFakeName(wchar_t* retValue) const
        {
            wchar_t* fakeName = retValue;

            *(__int64*)fakeName = 0;      //���8�ֽ�

            unsigned __int32 ordinalLow32 = this->Ordinal & 0x00000000FFFFFFFF;

            int charIndex = 0;
            do
            {
                unsigned long temp = ordinalLow32;
                temp &= 0x00003FFF;
                temp += 0x00005000;

                fakeName[charIndex] = temp & 0x0000FFFF;
                ++charIndex;

                ordinalLow32 >>= 0x0E;
            } while (ordinalLow32 != 0);
        }
    };

	class ExtractCore
	{
	private:
		static constexpr const char CreateStreamSignature[] = "\x55\x8B\xEC\x6A\xFF\x68\x2A\x2A\x2A\x2A\x64\xA1\x00\x00\x00\x00\x50\x51\xA1\x2A\x2A\x2A\x2A\x33\xC5\x50\x8D\x45\xF4\x64\xA3\x00\x00\x00\x00\xA1\x2A\x2A\x2A\x2A\x85\xC0\x75\x32\x68\xB0\x30\x00\x00";
		static constexpr const char CreateIndexSignature[] = "\x55\x8B\xEC\x6A\xFF\x68\x2A\x2A\x2A\x2A\x64\xA1\x00\x00\x00\x00\x50\x83\xEC\x14\x57\xA1\x2A\x2A\x2A\x2A\x33\xC5\x50\x8D\x45\xF4\x64\xA3\x00\x00\x00\x00\x83\x7D\x08\x00\x0F\x84\x2A\x2A\x00\x00\xA1\x2A\x2A\x2A\x2A\x85\xC0\x75\x12\x68\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x83\xC4\x04\xA3\x2A\x2A\x2A\x2A\xFF\x75\x0C\x8D\x4D\xF0\x51\xFF\xD0\xA1\x2A\x2A\x2A\x2A\xC7\x45\xFC\x00\x00\x00\x00\x85\xC0";
        static constexpr const wchar_t Split[] = L"##YSig##";

		using tCreateStream = IStream* (__cdecl*)(const tTJSString* fakeName, tjs_int64 key, tjs_uint32 encryptMode);
		using tCreateIndex = tjs_error (__cdecl*)(tTJSVariant* retValue, const tTJSVariant* tjsXP3Name);

		tCreateStream mCreateStreamFunc;		//CxCreateStream���ļ����ӿ�
		tCreateIndex mCreateIndexFunc;			//CxCreateIndex��ȡ�ļ���ӿ�

		std::wstring mExtractDirectoryPath;		//�������ļ���
        Log::Logger mLogger;

	public:
		ExtractCore();
		ExtractCore(const ExtractCore&) = delete;
		ExtractCore(ExtractCore&&) = delete;
        ExtractCore& operator=(const ExtractCore&) = delete;
        ExtractCore& operator=(ExtractCore&&) = delete;
        ~ExtractCore();

		/// <summary>
		/// ��������ļ���
		/// </summary>
		/// <param name="directory">�ļ��о���·��</param>
		void SetOutputDirectory(const std::wstring& directory);

        /// <summary>
        /// ������־���·��
        /// </summary>
        /// <param name="path">����·��</param>
        void SetLoggerPath(const std::wstring& path);

		/// <summary>
		/// ��ʼ�� (�������ҽӿ�)
		/// </summary>
		/// <param name="codeVa">������ʼ��ַ</param>
		/// <param name="codeSize">�����С</param>
		void Initialize(PVOID codeVa, DWORD codeSize);
		/// <summary>
		/// ����Ƿ��Ѿ���ʼ��
		/// </summary>
		/// <returns>True�ѳ�ʼ�� Falseδ��ʼ��</returns>
		bool IsInitialized();
		/// <summary>
		/// ���
		/// </summary>
		/// <param name="packageFileName">�������</param>
		void ExtractPackage(const std::wstring& packageFileName);

	private:
		/// <summary>
		/// ��ȡHxv4�ļ���
		/// </summary>
		/// <param name="xp3PackagePath">�������·��</param>
		/// <param name="retValue">�ļ�������</param>
		void GetEntries(const tTJSString& xp3PackagePath, std::vector<FileEntry>& retValue);

        /// <summary>
        /// ������Դ��
        /// </summary>
        /// <param name="entry">�ļ���</param>
        /// <param name="packageName">�����</param>
        /// <returns>IStream����</returns>
        IStream* CreateStream(const FileEntry& entry, const std::wstring& packageName);

        /// <summary>
        /// ��ȡ�ļ�
        /// </summary>
        /// <param name="stream">��</param>
        /// <param name="extractPath">��ȡ·��</param>
        void ExtractFile(IStream* stream, const std::wstring& extractPath);

        /// <summary>
        /// ���Խ����ı�
        /// </summary>
        /// <param name="stream">��Դ��</param>
        /// <param name="output">���������</param>
        /// <returns>True���ܳɹ� False�����ı�����</returns>
        static bool TryDecryptText(IStream* stream, std::vector<uint8_t>& output);
	};
}


