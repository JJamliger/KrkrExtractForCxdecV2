#include "ExtractCore.h"
#include "Base.h"
#include "pe.h"
#include "file.h"
#include "path.h"
#include "ExtendUtils.h"

namespace Engine
{
	//**********ExtractCore***********//
	ExtractCore::ExtractCore()
	{
		this->mCreateStreamFunc = nullptr;
		this->mCreateIndexFunc = nullptr;
	}

	ExtractCore::~ExtractCore()
	{
	}

	void ExtractCore::SetOutputDirectory(const std::wstring& directory)
	{
		this->mExtractDirectoryPath = directory;
	}

	void ExtractCore::SetLoggerPath(const std::wstring& path)
	{
		File::Delete(path);
		this->mLogger.Open(path.c_str());
	}

	void ExtractCore::Initialize(PVOID codeVa, DWORD codeSize)
	{
		PVOID createStream = PE::SearchPattern(codeVa, codeSize, ExtractCore::CreateStreamSignature, sizeof(ExtractCore::CreateStreamSignature) - 1);
		PVOID createIndex = PE::SearchPattern(codeVa, codeSize, ExtractCore::CreateIndexSignature, sizeof(ExtractCore::CreateIndexSignature) - 1);

		if (createStream && createIndex)
		{
			this->mCreateStreamFunc = (tCreateStream)createStream;
			this->mCreateIndexFunc = (tCreateIndex)createIndex;
		}
	}

	bool ExtractCore::IsInitialized()
	{
		return this->mCreateStreamFunc && this->mCreateIndexFunc;
	}

	void ExtractCore::ExtractPackage(const std::wstring& packageFileName)
	{
		if (!this->IsInitialized())
		{
			MessageBoxW(nullptr, L"δ��ʼ��CxdecV2�ӿ�\n�����Ƿ�Ϊ��DRM��Wamsoft Hxv4������Ϸ", L"����", MB_OK);
			return;
		}

		tTJSString tjsXp3PackagePath = TVPGetAppPath() + packageFileName.c_str();   //��ȡ�����׼·��
		std::vector<FileEntry> entries = std::vector<FileEntry>();

		this->GetEntries(tjsXp3PackagePath, entries);
		if (!entries.empty())
		{
			FullCreateDirectoryW(this->mExtractDirectoryPath);     //������Դ��ȡ�����ļ���

			std::wstring extractOutput = this->mExtractDirectoryPath + L"\\";	//�����ļ���
			std::wstring packageName = Path::GetFileNameWithoutExtension(packageFileName);   //�����
			std::wstring fileTableOutput = extractOutput + packageName + L".alst";      //�ļ������·��

			File::Delete(fileTableOutput);
			Log::Logger fileTable = Log::Logger(fileTableOutput.c_str());

			//дUTF-16LE bomͷ
			{
				WORD bom = 0xFEFF;
				fileTable.WriteData(&bom, sizeof(bom));
			}

			for (FileEntry& entry : entries)
			{
				std::wstring dirHash = BytesToHexStringW(entry.DirectoryPathHash, sizeof(entry.DirectoryPathHash));  //�ļ���Hash�ַ�����ʽ
				std::wstring fileNameHash = BytesToHexStringW(entry.FileNameHash, sizeof(entry.FileNameHash));     //�ļ���Hash�ַ�����ʽ
				std::wstring arcOutputPath = extractOutput + packageName + L"\\" + dirHash + L"\\" + fileNameHash;	//����Դ����·��

				if (IStream* stream = this->CreateStream(entry, packageFileName))
				{
					//д��  dirHash[Sign]dirHash[Sign]fileHash[Sign]fileHash[NewLine]
					fileTable.WriteUnicode(L"%s%s%s%s%s%s%s\r\n",
										   dirHash.c_str(),
										   ExtractCore::Split,
										   dirHash.c_str(),
										   ExtractCore::Split,
										   fileNameHash.c_str(),
										   ExtractCore::Split,
										   fileNameHash.c_str());
					//���
					this->ExtractFile(stream, arcOutputPath);
					stream->Release();
				}
				else
				{
					this->mLogger.WriteLine(L"File Not Exist: %s/%s/%s", packageName.c_str(), dirHash.c_str(), fileNameHash.c_str());
				}
			}
			MessageBoxW(nullptr, (packageFileName + L"��ȡ�ɹ�").c_str(), L"��ʾ", MB_OK);
			fileTable.Close();
		}
		else
		{
			MessageBoxW(nullptr, L"��ѡ����ȷ��XP3���", L"����", MB_OK);
		}
	}

	void ExtractCore::GetEntries(const tTJSString& xp3PackagePath, std::vector<FileEntry>& retValue)
	{
		retValue.clear();

		//�����ļ���
		tTJSVariant tjsEntries = tTJSVariant();
		tTJSVariant tjsPackagePath = tTJSVariant(xp3PackagePath);
		this->mCreateIndexFunc(&tjsEntries, &tjsPackagePath);

		if (tjsEntries.Type() == tvtObject)
		{
			tTJSVariantClosure& dirEntriesObj = tjsEntries.AsObjectClosureNoAddRef();

			//��ȡ��������
			tTJSVariant tjsCount = tTJSVariant();
			tjs_int count = 0;
			dirEntriesObj.PropGet(TJS_MEMBERMUSTEXIST, L"count", NULL, &tjsCount, nullptr);
			count = tjsCount.AsInteger();

			constexpr tjs_int DirectoryItemSize = 2;
			//��ȡ�ļ������� (ռKR����2��)
			//�ļ���·��Hash
			//���ļ�����
			tjs_int dirCount = count / DirectoryItemSize;

			//�����ļ���
			for (tjs_int di = 0; di < dirCount; ++di)
			{
				//��ȡ�ļ���·��Hash�����ļ���
				tTJSVariant tjsDirHash = tTJSVariant();
				tTJSVariant tjsFileEntries = tTJSVariant();
				dirEntriesObj.PropGetByNum(TJS_CII_GET, di * DirectoryItemSize + 0, &tjsDirHash, nullptr);
				dirEntriesObj.PropGetByNum(TJS_CII_GET, di * DirectoryItemSize + 1, &tjsFileEntries, nullptr);

				//��ȡHash
				tTJSVariantOctet* dirHash = tjsDirHash.AsOctetNoAddRef();

				//��ȡ���ļ�������
				tTJSVariantClosure& fileEntries = tjsFileEntries.AsObjectClosureNoAddRef();

				//��ȡ���ļ���������
				tjsCount.Clear();
				fileEntries.PropGet(TJS_MEMBERMUSTEXIST, L"count", NULL, &tjsCount, nullptr);
				count = tjsCount.AsInteger();

				constexpr tjs_int FileItemSize = 2;
				//��ȡ�ļ����� (ռKR����2��)
				//�ļ���Hash
				//�ļ���Ϣ
				tjs_int fileCount = count / FileItemSize;

				//�������ļ�
				for (tjs_int fi = 0; fi < fileCount; ++fi)
				{
					//��ȡ�ļ���Hash���ļ���Ϣ
					tTJSVariant tjsFileNameHash = tTJSVariant();
					tTJSVariant tjsFileInfo = tTJSVariant();
					fileEntries.PropGetByNum(TJS_CII_GET, fi * FileItemSize + 0, &tjsFileNameHash, nullptr);
					fileEntries.PropGetByNum(TJS_CII_GET, fi * FileItemSize + 1, &tjsFileInfo, nullptr);

					//��ȡHash
					tTJSVariantOctet* fileNameHash = tjsFileNameHash.AsOctetNoAddRef();

					//��ȡ�ļ���Ϣ
					tTJSVariantClosure& fileInfo = tjsFileInfo.AsObjectClosureNoAddRef();

					//��ȡ�ļ���Ϣ
					__int64 ordinal = 0;
					__int64 key = 0;

					tTJSVariant tjsValue = tTJSVariant();
					fileInfo.PropGetByNum(TJS_CII_GET, 0, &tjsValue, nullptr);
					ordinal = tjsValue.AsInteger();

					tjsValue.Clear();
					fileInfo.PropGetByNum(TJS_CII_GET, 1, &tjsValue, nullptr);
					key = tjsValue.AsInteger();

					//��������ļ���
					FileEntry entry{ 0 };
					memcpy(entry.DirectoryPathHash, dirHash->GetData(), dirHash->GetLength());
					memcpy(entry.FileNameHash, fileNameHash->GetData(), fileNameHash->GetLength());

					entry.Key = key;
					entry.Ordinal = ordinal;

					retValue.push_back(entry);
				}
			}
		}
	}

	IStream* ExtractCore::CreateStream(const FileEntry& entry, const std::wstring& packageName)
	{
		tjs_char fakeName[4]{ 0 };
		entry.GetFakeName(fakeName);

		tTJSString tjsArcPath = TVPGetAppPath();       //��ȡ��Ϸ·��
		tjsArcPath += packageName.c_str();     //��ӷ��
		tjsArcPath += L">";                    //��ӷ���ָ���
		tjsArcPath += fakeName;    //�����Դ��
		tjsArcPath = TVPNormalizeStorageName(tjsArcPath);

		return this->mCreateStreamFunc(&tjsArcPath, entry.Key, entry.GetEncryptMode());
	}

	void ExtractCore::ExtractFile(IStream* stream, const std::wstring& extractPath)
	{
		unsigned long long size = StreamUtils::IStreamEx::Length(stream);	//��ȡ������

		//���·��
		std::wstring relativePath(&extractPath.c_str()[this->mExtractDirectoryPath.length() + 1]);
		if (size > 0)
		{
			//�����ļ���
			{
				std::wstring outputDir = Path::GetDirectoryName(extractPath);
				if (!outputDir.empty())
				{
					FullCreateDirectoryW(outputDir.c_str());
				}
			}

			std::vector<uint8_t> buffer;
			bool success = false;

			if (ExtractCore::TryDecryptText(stream, buffer))  //���Խ���SimpleEncrypt
			{
				success = true;
			}
			else
			{
				//��ͨ��Դ
				buffer.resize(size);

				stream->Seek(LARGE_INTEGER{ 0 }, STREAM_SEEK_SET, NULL);
				if (StreamUtils::IStreamEx::Read(stream, buffer.data(), size) == size)
				{
					success = true;
				}
			}

			if (success && !buffer.empty())
			{
				//�����ļ�
				if (File::WriteAllBytes(extractPath, buffer.data(), buffer.size()))
				{
					this->mLogger.WriteLine(L"Extract Successed: %s", relativePath.c_str());
				}
				else
				{
					this->mLogger.WriteLine(L"Write Error: %s", relativePath.c_str());
				}
			}
			else
			{
				this->mLogger.WriteLine(L"Invaild File: %s", relativePath.c_str());
			}
			stream->Seek(LARGE_INTEGER{ 0 }, STREAM_SEEK_SET, NULL);
		}
		else
		{
			this->mLogger.WriteLine(L"Empty File: %s", relativePath.c_str());
		}
	}


	bool ExtractCore::TryDecryptText(IStream* stream, std::vector<uint8_t>& output)
	{
		uint8_t mark[2]{ 0 };
		StreamUtils::IStreamEx::Read(stream, mark, 2);

		if (mark[0] == 0xfe && mark[1] == 0xfe)   //�����ܱ��ͷ
		{
			uint8_t mode;
			StreamUtils::IStreamEx::Read(stream, &mode, 1);

			if (mode != 0 && mode != 1 && mode != 2)  //ʶ��ģʽ
			{
				return false;
			}

			ZeroMemory(mark, sizeof(mark));
			StreamUtils::IStreamEx::Read(stream, mark, 2);

			if (mark[0] != 0xff || mark[1] != 0xfe)  //Unicode Bom
			{
				return false;
			}

			if (mode == 2)   //ѹ��ģʽ
			{
				long long compressed = 0;
				long long uncompressed = 0;
				StreamUtils::IStreamEx::Read(stream, &compressed, sizeof(long long));
				StreamUtils::IStreamEx::Read(stream, &uncompressed, sizeof(long long));

				if (compressed <= 0 || compressed >= INT_MAX || uncompressed <= 0 || uncompressed >= INT_MAX)
				{
					return false;
				}

				std::vector<uint8_t> data = std::vector<uint8_t>((size_t)compressed);

				//��ȡѹ������
				if (StreamUtils::IStreamEx::Read(stream, data.data(), compressed) != compressed)
				{
					return false;
				}

				std::vector<uint8_t> buffer = std::vector<uint8_t>((size_t)uncompressed + 2);

				//д��Bomͷ
				buffer[0] = mark[0];
				buffer[1] = mark[1];

				uint8_t* dest = buffer.data() + 2;
				unsigned long destLen = (unsigned long)uncompressed;

				if (ZLIB_uncompress(dest, &destLen, data.data(), compressed) || destLen != (unsigned long)uncompressed)
				{
					return false;
				}

				output = std::move(buffer);
				return true;
			}
			else
			{
				long long startpos = StreamUtils::IStreamEx::Position(stream); //������ʼλ��
				long long endpos = StreamUtils::IStreamEx::Length(stream); //���ܽ���λ��

				StreamUtils::IStreamEx::Seek(stream, startpos, STREAM_SEEK_SET);      //���û���ʼλ��

				long long size = endpos - startpos;   //���ܴ�С

				if (size <= 0 || size >= INT_MAX)
				{
					return false;
				}

				size_t count = (size_t)(size / sizeof(wchar_t));

				if (count == 0)
				{
					return false;
				}

				std::vector<wchar_t> buffer(count);  //����ı�

				StreamUtils::IStreamEx::Read(stream, buffer.data(), size);  //��ȡ��Դ

				if (mode == 0)  //ģʽ0
				{
					for (size_t i = 0; i < count; i++)
					{
						wchar_t ch = buffer[i];
						if (ch >= 0x20) buffer[i] = ch ^ (((ch & 0xfe) << 8) ^ 1);
					}
				}
				else if (mode == 1)   //ģʽ1
				{
					for (size_t i = 0; i < count; i++)
					{
						wchar_t ch = buffer[i];
						ch = ((ch & 0xaaaaaaaa) >> 1) | ((ch & 0x55555555) << 1);
						buffer[i] = ch;
					}
				}

				size_t sizeToCopy = count * sizeof(wchar_t);

				output.resize(sizeToCopy + 2);

				//д��Unicode Bom
				output[0] = mark[0];
				output[1] = mark[1];

				//��д���ܺ������
				memcpy(output.data() + 2, buffer.data(), sizeToCopy);

				return true;
			}
		}
		return false;
	}
	//================================//
}
