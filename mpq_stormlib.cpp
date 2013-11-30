#include <vector>
#include <string>
#include "wx/file.h"
#include "wx/log.h"
#include "wx/dir.h"
#include "mpq_stormlib.h"
#include "util.h"
//#include "globalvars.h"

using namespace std;


typedef vector< pair< wxString, HANDLE* > > ArchiveSet;
//所有已经打开的mpq
static ArchiveSet gOpenArchives;

//这里定义GamePath为"d:\World of Warcraft"
wxString gamePath = "d:\\World of Warcraft\\Data\\";
//wxString gamePath = "E:\\wowtw\\Data\\";

//wxString gamePath = "d:\\";


//wow客户端的语言信息
long langID = -1;
//所有的文件名信息
wxArrayString mpqArchives;
//语言名称 比如说"zhTW"
wxString langName;

bool bAlternate = false; // for zhCN alternate.MPQ  这个是不是简体客户端的一个特例？还没有明白

//这个不清楚 应该是通过手动设置的一个选项，比如说从Settings文件或者 一个控件中得到的结果把
bool useLocalFiles = false;

//这里是遍历和语言相关的MPQ吧 traversLocaleMPQs来自于app.cpp
long traverseLocaleMPQs(const wxString locales[], size_t localeCount, const wxString localeArchives[], size_t archiveCount, const wxString& gamePath)
{
	printf("111");
	long lngID = -1;

	for (size_t i = 0; i < localeCount; i++) {
		if (locales[i].IsEmpty())
			continue;
		wxString localePath = gamePath;

		localePath.Append(locales[i]);
		localePath.Append(wxT("/"));
		if (wxDir::Exists(localePath)) {
			wxArrayString localeMpqs;
			wxDir::GetAllFiles(localePath, &localeMpqs, wxEmptyString, wxDIR_FILES);

			for (size_t j = 0; j < archiveCount; j++) {
				for (size_t k = 0; k < localeMpqs.size(); k++) {
					wxString baseName = wxFileName(localeMpqs[k]).GetFullName();
					wxString neededMpq = wxString::Format(localeArchives[j], locales[i].c_str());

					if(baseName.CmpNoCase(neededMpq) == 0) {
						mpqArchives.Add(localeMpqs[k]);
					}
				}
			}

			lngID = (long)i;
			return lngID;
		}
	}

	return lngID;
}

//查找所有的MPQ来自 app.cpp
void searchMPQs(bool firstTime)
{
	if (mpqArchives.GetCount() > 0)
		return;

	bool bSearchCache = false;
	//wxMessageDialog *dial = new wxMessageDialog(NULL, _("Do you want to search Cache dir?"),
		//_("Question"), wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);
	//if (wxID_YES == dial->ShowModal())
		//bSearchCache = true;

	const wxString locales[] = {
		// sets 0
		wxT("enUS"), wxT("koKR"), wxT("frFR"), wxT("deDE"), 
		wxT("zhCN"), wxT("zhTW"), wxT("esES"), wxT("esMX"), wxT("ruRU"),
		wxT("jaJP"), wxT("ptBR"),
		// sets 1
		wxT("enGB"), wxEmptyString, wxEmptyString, wxEmptyString, 
		wxT("enCN"), wxT("enTW"), wxEmptyString, wxEmptyString, wxEmptyString,
		wxEmptyString, wxEmptyString
		};
	const int localeSets = WXSIZEOF(locales) / 2;
	const wxString defaultArchives[] = {wxT("patch-9.mpq"),wxT("patch-8.mpq"),wxT("patch-7.mpq"),wxT("patch-6.mpq"),
		wxT("patch-5.mpq"),wxT("patch-4.mpq"),wxT("patch-3.mpq"),wxT("patch-2.mpq"),wxT("patch.mpq"),wxT("alternate.mpq"),
		wxT("expansion3.mpq"),wxT("expansion2.mpq"),wxT("expansion1.mpq"),wxT("lichking.mpq"),wxT("expansion.mpq"),
		wxT("world.mpq"),wxT("sound.mpq"),wxT("art.mpq"),wxT("common-3.mpq"),wxT("common-2.mpq"), wxT("common.mpq")};
	const wxString localeArchives[] = {wxT("patch-%s-9.mpq"),wxT("patch-%s-8.mpq"),wxT("patch-%s-7.mpq"),
		wxT("patch-%s-6.mpq"),wxT("patch-%s-5.mpq"),wxT("patch-%s-4.mpq"),wxT("patch-%s-3.mpq"), wxT("patch-%s-2.mpq"), 
		wxT("patch-%s.mpq"), wxT("expansion3-locale-%s.mpq"), wxT("expansion2-locale-%s.mpq"), 
		wxT("expansion1-locale-%s.mpq"), wxT("lichking-locale-%s.mpq"), wxT("expansion-locale-%s.mpq"), 
		wxT("locale-%s.mpq"), wxT("base-%s.mpq")};

	// select avaiable locales, auto select user config locale
	wxArrayString avaiLocales;
	for (size_t i = 0; i < WXSIZEOF(locales); i++) {
		if (locales[i].IsEmpty())
			continue;
		wxString localePath = gamePath + wxT("Cache") + SLASH + locales[i];
		if (wxDir::Exists(localePath))
			avaiLocales.Add(locales[i]);
	}
	/*Hack For Shengli */
	langName = "zhTW";
	/*if (firstTime && avaiLocales.size() == 1) // only 1 locale
		langName = avaiLocales[0];
	else {
		// if user never select a locale, show all locales in data directory
		avaiLocales.Clear();
		for (size_t i = 0; i < WXSIZEOF(locales); i++) {
			if (locales[i].IsEmpty())
				continue;
			wxString localePath = gamePath + locales[i];

			if (wxDir::Exists(localePath))
				avaiLocales.Add(locales[i]);
		}
		if (avaiLocales.size() == 0) // failed to find locale
			return;
		else if (avaiLocales.size() == 1) // only 1 locale
			langName = avaiLocales[0];
		else{
			printf("Here we got the fuck\n");
			langName = wxGetSingleChoice(_("Please select a Locale:"), _("Locale"), avaiLocales);
		}
	}*/

	// search Partial MPQs
	wxArrayString baseMpqs;
	wxDir::GetAllFiles(gamePath, &baseMpqs, wxEmptyString, wxDIR_FILES);
	for (size_t j = 0; j < baseMpqs.size(); j++) {
		if (baseMpqs[j].Contains(wxT("oldworld")))
			continue;
		wxString baseName = wxFileName(baseMpqs[j]).GetFullName();
		wxString cmpName = wxT("wow-update-");
		if (baseName.StartsWith(cmpName) && baseName.AfterLast('.').CmpNoCase(wxT("mpq")) == 0) {
			bool bFound = false;
			for(size_t i = 0; i<mpqArchives.size(); i++) {
				wxString archiveName = wxFileName(mpqArchives[i]).GetFullName();
				if (!archiveName.AfterLast(SLASH).StartsWith(cmpName))
					continue;
				int ver = wxAtoi(archiveName.BeforeLast('.').AfterLast('-'));
				int bver = wxAtoi(baseName.BeforeLast('.').AfterLast('-'));
				if (bver > ver) {
					mpqArchives.Insert(baseMpqs[j], i);
					bFound = true;
					break;
				}		
			}
			if (bFound == false)
				mpqArchives.Add(baseMpqs[j]);

			wxLogMessage(wxT("- Found Partial MPQ archive: %s"), baseMpqs[j].Mid(gamePath.Len()).c_str());
		}
	}

	// search Partial MPQs inside langName directory
	wxDir::GetAllFiles(gamePath+langName, &baseMpqs, wxEmptyString, wxDIR_FILES);
	for (size_t j = 0; j < baseMpqs.size(); j++) {
		if (baseMpqs[j].Contains(wxT("oldworld")))
			continue;
		wxString baseName = wxFileName(baseMpqs[j]).GetFullName();
		wxString cmpName = wxT("wow-update-")+langName;
		if (baseName.StartsWith(cmpName) && baseName.AfterLast('.').CmpNoCase(wxT("mpq")) == 0) {
			bool bFound = false;
			for(size_t i = 0; i<mpqArchives.size(); i++) {
				wxString archiveName = wxFileName(mpqArchives[i]).GetFullName();
				if (!archiveName.StartsWith(wxT("wow-update-"))) // compare to all wow-update-
					continue;
				int ver = wxAtoi(archiveName.BeforeLast('.').AfterLast('-'));
				int bver = wxAtoi(baseName.BeforeLast('.').AfterLast('-'));
				if (bver > ver) {
					mpqArchives.Insert(baseMpqs[j], i);
					bFound = true;
					break;
				}		
			}
			if (bFound == false)
				mpqArchives.Add(baseMpqs[j]);

			wxLogMessage(wxT("- Found Partial MPQ archive: %s"), baseMpqs[j].Mid(gamePath.Len()).c_str());
		}
	}

	// search patch-base MPQs
	wxArrayString baseCacheMpqs;
	wxDir::GetAllFiles(gamePath+wxT("Cache"), &baseCacheMpqs, wxEmptyString, wxDIR_FILES);
	for (size_t j = 0; j < baseCacheMpqs.size(); j++) {
		if (bSearchCache == false)
			continue;
		if (baseCacheMpqs[j].Contains(wxT("oldworld")))
			continue;
		wxString baseName = baseCacheMpqs[j];
		wxString fullName = wxFileName(baseName).GetFullName();
		wxString cmpName = wxT("patch-base-");
		if (fullName.StartsWith(cmpName) && fullName.AfterLast('.').CmpNoCase(wxT("mpq")) == 0) {
			bool bFound = false;
			for(size_t i = 0; i<mpqArchives.size(); i++) {
				if (!mpqArchives[i].AfterLast(SLASH).StartsWith(cmpName))
					continue;
				int ver = wxAtoi(mpqArchives[i].BeforeLast('.').AfterLast('-'));
				int bver = wxAtoi(fullName.BeforeLast('.').AfterLast('-'));
				if (bver > ver) {
#if 1 // Use lastest archive only
					mpqArchives[i] = baseName;
#else
					mpqArchives.Insert(baseName, i);
#endif
					bFound = true;
					break;
				}		
			}
			if (bFound == false)
				mpqArchives.Add(baseName);

			wxLogMessage(wxT("- Found Patch Base MPQ archive: %s"), baseName.Mid(gamePath.Len()).c_str());
		}
	}
	baseCacheMpqs.Clear();

	// search base cache locale MPQs
	wxArrayString baseCacheLocaleMpqs;
	wxDir::GetAllFiles(gamePath+wxT("Cache")+SLASH+langName, &baseCacheLocaleMpqs, wxEmptyString, wxDIR_FILES);
	for (size_t j = 0; j < baseCacheLocaleMpqs.size(); j++) {
		if (bSearchCache == false)
			continue;
		if (baseCacheLocaleMpqs[j].Contains(wxT("oldworld")))
			continue;
		wxString baseName = baseCacheLocaleMpqs[j];
		wxString fullName = wxFileName(baseName).GetFullName();
		wxString cmpName = wxT("patch-")+langName+wxT("-");
		if (fullName.StartsWith(cmpName) && fullName.AfterLast('.').CmpNoCase(wxT("mpq")) == 0) {
			bool bFound = false;
			for(size_t i = 0; i<mpqArchives.size(); i++) {
				if (!mpqArchives[i].AfterLast(SLASH).StartsWith(cmpName))
					continue;
				int ver = wxAtoi(mpqArchives[i].BeforeLast('.').AfterLast('-'));
				int bver = wxAtoi(fullName.BeforeLast('.').AfterLast('-'));
				if (bver > ver) {
#if 1 // Use lastest archive only
					mpqArchives[i] = baseName;
#else
					mpqArchives.Insert(baseName, i);
#endif
					bFound = true;
					break;
				}		
			}
			if (bFound == false)
				mpqArchives.Add(baseName);

			wxLogMessage(wxT("- Found Patch Base Locale MPQ archive: %s"), baseName.Mid(gamePath.Len()).c_str());
		}
	}
	baseCacheLocaleMpqs.Clear();

	// default archives
	for (size_t i = 0; i < WXSIZEOF(defaultArchives); i++) {
		//wxLogMessage(wxT("Searching for MPQ archive %s..."), defaultArchives[i].c_str());

		for (size_t j = 0; j < baseMpqs.size(); j++) {
			wxString baseName = wxFileName(baseMpqs[j]).GetFullName();
			if(baseName.CmpNoCase(defaultArchives[i]) == 0) {
				mpqArchives.Add(baseMpqs[j]);

				wxLogMessage(wxT("- Found MPQ archive: %s"), baseMpqs[j].Mid(gamePath.Len()).c_str());
				if (baseName.CmpNoCase(wxT("alternate.mpq")))
					bAlternate = true;
			}
		}
	}

	// add locale files
	for (size_t i = 0; i < WXSIZEOF(locales); i++) {
		if (locales[i] == langName) {
			wxString localePath = gamePath;

			localePath.Append(locales[i]);
			localePath.Append(wxT("/"));
			if (wxDir::Exists(localePath)) {
				wxArrayString localeMpqs;
				wxDir::GetAllFiles(localePath, &localeMpqs, wxEmptyString, wxDIR_FILES);

				for (size_t j = 0; j < WXSIZEOF(localeArchives); j++) {
					for (size_t k = 0; k < localeMpqs.size(); k++) {
						wxString baseName = wxFileName(localeMpqs[k]).GetFullName();
						wxString neededMpq = wxString::Format(localeArchives[j], locales[i].c_str());

						if(baseName.CmpNoCase(neededMpq) == 0) {
							mpqArchives.Add(localeMpqs[k]);
						}
					}
				}
			}

			langID = i % localeSets;
			break;
		}
	}

	if (langID == -1) {
		langID = traverseLocaleMPQs(locales, WXSIZEOF(locales), localeArchives, WXSIZEOF(localeArchives), gamePath);
		if (langID != -1)
			langID = langID % localeSets;
	}
	//这里打开所有的MPQ文件 我们这里只加载art.mpq
	for(int i = 0;i < mpqArchives.GetCount();i++){
		//if(mpqArchives[i].Matches("*World*") ||mpqArchives[i].Matches("*art*") || mpqArchives[i].Matches("*locale-zhTW*") || mpqArchives[i].Matches("*expansion3*") || mpqArchives[i].Matches("*expansion2*")){
		if(mpqArchives[i].Matches("*world2.MPQ") || mpqArchives[i].Matches("*world.MPQ")||mpqArchives[i].Matches("*art*") || mpqArchives[i].Matches("*locale-zhTW*")){
			std::cout<<mpqArchives[i].c_str() <<std::endl;
			new MPQArchive(mpqArchives[i] );	
		}
	}

}



MPQArchive::MPQArchive(wxString filename) : ok(false)
{
	wxLogMessage(wxT("Opening %s %s"), filename.Mid(gamePath.Len()).c_str(), isPartialMPQ(filename) ? "(Partial)" : "");
//	g_modelViewer->SetStatusText(wxT("Initiating "+filename+wxT(" Archive")));
#ifndef _MINGW
	if (!SFileOpenArchive(filename.fn_str(), 0, MPQ_OPEN_FORCE_MPQ_V1|MPQ_OPEN_READ_ONLY, &mpq_a )) {
#else
	if (!SFileOpenArchive(filename.char_str(), 0, MPQ_OPEN_FORCE_MPQ_V1|MPQ_OPEN_READ_ONLY, &mpq_a )) {
#endif
		int nError = GetLastError();
		wxLogMessage(wxT("Error opening archive %s, error #: 0x%X"), filename.Mid(gamePath.Len()).c_str(), nError);
		return;
	}

	
	// do patch, but skip cache\ directory
	if (!(filename.BeforeLast(SLASH).Lower().Contains(wxT("cache")) && 
		filename.AfterLast(SLASH).Lower().StartsWith(wxT("patch"))) &&
		!isPartialMPQ(filename)) { // skip the PTCH files atrchives
		// do patch
		for(ssize_t j=mpqArchives.GetCount()-1; j>=0; j--) {
			if (!mpqArchives[j].AfterLast(SLASH).StartsWith(wxT("wow-update-")))
				continue;
			if (mpqArchives[j].AfterLast(SLASH).Len() == strlen("wow-update-xxxxx.mpq")) {
#ifndef _MINGW
				SFileOpenPatchArchive(mpq_a, mpqArchives[j].fn_str(), "base", 0);
				SFileOpenPatchArchive(mpq_a, mpqArchives[j].fn_str(), langName.fn_str(), 0);
#else
				SFileOpenPatchArchive(mpq_a, mpqArchives[j].char_str(), "base", 0);
				SFileOpenPatchArchive(mpq_a, mpqArchives[j].char_str(), langName.char_str(), 0);
#endif
				// too many for ptr client, just comment it
				// wxLogMessage(wxT("Appending base & %s patch %s"), langName.c_str(), mpqArchives[j].Mid(gamePath.Len()).c_str());
			} else if (mpqArchives[j].BeforeLast(SLASH) == filename.BeforeLast(SLASH)) { // same directory only
#ifndef _MINGW
				SFileOpenPatchArchive(mpq_a, mpqArchives[j].fn_str(), "", 0);
#else
				SFileOpenPatchArchive(mpq_a, mpqArchives[j].char_str(), "", 0);
#endif
				// wxLogMessage(wxT("Appending patch %s"), mpqArchives[j].Mid(gamePath.Len()).c_str());
			}
		}
	}

	ok = true;
	gOpenArchives.push_back( make_pair( filename, &mpq_a ) );
}

MPQArchive::~MPQArchive()
{
	/*
	for(ArchiveSet::iterator i=gOpenArchives.begin(); i!=gOpenArchives.end();++i)
	{
		mpq_archive &mpq_a = **i;
		
		free(mpq_a.header);
	}
	*/
	//gOpenArchives.erase(gOpenArchives.begin(), gOpenArchives.end());
}

bool MPQArchive::isPartialMPQ(wxString filename)
{
	if (filename.AfterLast(SLASH).StartsWith(wxT("wow-update-")))
		return true;
	return false;
}

void MPQArchive::close()
{
	if (ok == false)
		return;
	SFileCloseArchive(mpq_a);
	for(ArchiveSet::iterator it=gOpenArchives.begin(); it!=gOpenArchives.end();++it)
	{
		HANDLE &mpq_b = *it->second;
		if (&mpq_b == &mpq_a) {
			gOpenArchives.erase(it);
			//delete (*it);
			return;
		}
	}
	
}

bool MPQFile::isPartialMPQ(wxString filename)
{
	if (filename.AfterLast(SLASH).StartsWith(wxT("wow-update-")))
		return true;
	return false;
}

void
MPQFile::openFile(wxString filename)
{
	eof = false;
	buffer = 0;
	pointer = 0;
	size = 0;
	if( useLocalFiles ) {
		wxString fn1 = wxGetCwd()+SLASH+wxT("Import")+SLASH;
		wxString fn2 = fn1;
		wxString fn3 = gamePath;
		fn1.Append(filename);
		fn2.Append(filename.AfterLast(SLASH));
		fn3.Append(filename);

		wxString fns[] = { fn1, fn2, fn3 };
		for(size_t i=0; i<WXSIZEOF(fns); i++) {
			wxString fn = fns[i];
			if (wxFile::Exists(fn)) {
				// success
				wxFile file;
				// if successfully opened
				if (file.Open(fn, wxFile::read)) {
					size = file.Length();
					if (size > 0) {
						buffer = new unsigned char[size];
						// if successfully read data
						if (file.Read(buffer, size) > 0) {
							eof = false;
							file.Close();
							return;
						} else {
							wxDELETEA(buffer);
							eof = true;
							size = 0;
						}
					}
					file.Close();
				}
			}
		}
	}

	// zhCN alternate file mode
	if (bAlternate && !filename.Lower().StartsWith(wxT("alternate"))) {
		wxString alterName = wxT("alternate")+SLASH+filename;

		for(ArchiveSet::iterator i=gOpenArchives.begin(); i!=gOpenArchives.end(); ++i)
		{
			HANDLE &mpq_a = *i->second;

			HANDLE fh;
#ifndef _MINGW
			if( !SFileOpenFileEx( mpq_a, alterName.fn_str(), SFILE_OPEN_PATCHED_FILE, &fh ) )
#else
			if( !SFileOpenFileEx( mpq_a, alterName.char_str(), SFILE_OPEN_PATCHED_FILE, &fh ) )
#endif
				continue;

			// Found!
			DWORD filesize = SFileGetFileSize( fh );
			size = filesize;

			// HACK: in patch.mpq some files don't want to open and give 1 for filesize
			if (size<=1) {
				eof = true;
				buffer = 0;
				return;
			}

			buffer = new unsigned char[size];
			SFileReadFile( fh, buffer, (DWORD)size );
			SFileCloseFile( fh );

			return;
		}
	}

	for(ArchiveSet::iterator i=gOpenArchives.begin(); i!=gOpenArchives.end(); ++i)
	{
		HANDLE &mpq_a = *i->second;

		HANDLE fh;
#ifndef _MINGW
		if( !SFileOpenFileEx( mpq_a, filename.fn_str(), SFILE_OPEN_PATCHED_FILE, &fh ) )
#else
		if( !SFileOpenFileEx( mpq_a, filename.char_str(), SFILE_OPEN_PATCHED_FILE, &fh ) )
#endif
			continue;

		// Found!
		DWORD filesize = SFileGetFileSize( fh );
		size = filesize;

		// HACK: in patch.mpq some files don't want to open and give 1 for filesize
		if (size<=1) {
			eof = true;
			buffer = 0;
			return;
		}

		buffer = new unsigned char[size];
		SFileReadFile( fh, buffer, (DWORD)size );
		SFileCloseFile( fh );

		return;
	}

	eof = true;
	buffer = 0;
}

MPQFile::MPQFile(wxString filename):
	eof(false),
	buffer(0),
	pointer(0),
	size(0)
{
	openFile(filename);
}

MPQFile::~MPQFile()
{
	close();
}

bool MPQFile::exists(wxString filename)
{
	if( useLocalFiles ) {
		wxString fn1 = wxGetCwd()+SLASH+wxT("Import")+SLASH;
		wxString fn2 = fn1;
		wxString fn3 = gamePath;
		fn1.Append(filename);
		fn2.Append(filename.AfterLast(SLASH));
		fn3.Append(filename);

		wxString fns[] = { fn1, fn2, fn3 };
		for(size_t i=0; i<WXSIZEOF(fns); i++) {
			wxString fn = fns[i];
			if (wxFile::Exists(fn))
				return true;
		}
	}

	for(ArchiveSet::iterator i=gOpenArchives.begin(); i!=gOpenArchives.end();++i)
	{
		HANDLE &mpq_a = *i->second;
#ifndef _MINGW
		if( SFileHasFile( mpq_a, filename.fn_str() ) )
#else
		if( SFileHasFile( mpq_a, filename.char_str() ) )
#endif
			return true;
	}

	return false;
}

void MPQFile::save(wxString filename)
{
	wxFile f;
	f.Open(filename, wxFile::write);
	f.Write(buffer, size);
	f.Close();
}

size_t MPQFile::read(void* dest, size_t bytes)
{
	if (eof) 
		return 0;

	size_t rpos = pointer + bytes;
	if (rpos > size) {
		bytes = size - pointer;
		eof = true;
	}

	memcpy(dest, &(buffer[pointer]), bytes);

	pointer = rpos;

	return bytes;
}

bool MPQFile::isEof()
{
    return eof;
}

void MPQFile::seek(ssize_t offset) {
	pointer = offset;
	eof = (pointer >= size);
}

void MPQFile::seekRelative(ssize_t offset)
{
	pointer += offset;
	eof = (pointer >= size);
}

void MPQFile::close()
{
	wxDELETEA(buffer);
	eof = true;
}

size_t MPQFile::getSize()
{
	return size;
}

int MPQFile::getSize(wxString filename)
{
	if( useLocalFiles ) {
		wxString fn1 = wxGetCwd()+SLASH+wxT("Import")+SLASH;
		wxString fn2 = fn1;
		wxString fn3 = gamePath;
		fn1.Append(filename);
		fn2.Append(filename.AfterLast(SLASH));
		fn3.Append(filename);

		wxString fns[] = { fn1, fn2, fn3 };
		for(size_t i=0; i<WXSIZEOF(fns); i++) {
			wxString fn = fns[i];
			if (wxFile::Exists(fn)) {
				wxFile file(fn);
				return file.Length();
			}
		}
	}

	for(ArchiveSet::iterator i=gOpenArchives.begin(); i!=gOpenArchives.end();++i)
	{
		HANDLE &mpq_a = *i->second;
		HANDLE fh;
#ifndef _MINGW
		if( !SFileOpenFileEx( mpq_a, filename.fn_str(), SFILE_OPEN_PATCHED_FILE, &fh ) )
#else
		if( !SFileOpenFileEx( mpq_a, filename.char_str(), SFILE_OPEN_PATCHED_FILE, &fh ) )
#endif
			continue;

		DWORD filesize = SFileGetFileSize( fh );
		SFileCloseFile( fh );
		return filesize;
	}

	return 0;
}

wxString MPQFile::getArchive(wxString filename)
{
	if( useLocalFiles ) {
		wxString fn1 = wxGetCwd()+SLASH+wxT("Import")+SLASH;
		wxString fn2 = fn1;
		wxString fn3 = gamePath;
		fn1.Append(filename);
		fn2.Append(filename.AfterLast(SLASH));
		fn3.Append(filename);

		wxString fns[] = { fn1, fn2, fn3 };
		for(size_t i=0; i<WXSIZEOF(fns); i++) {
			wxString fn = fns[i];
			if (wxFile::Exists(fn)) {
				return fn;
			}
		}
	}

	for(ArchiveSet::iterator i=gOpenArchives.begin(); i!=gOpenArchives.end();++i)
	{
		HANDLE &mpq_a = *i->second;
		HANDLE fh;
#ifndef _MINGW
		if( !SFileOpenFileEx( mpq_a, filename.fn_str(), SFILE_OPEN_PATCHED_FILE, &fh ) )
#else
		if( !SFileOpenFileEx( mpq_a, filename.char_str(), SFILE_OPEN_PATCHED_FILE, &fh ) )
#endif
			continue;

		return i->first;
	}

	return wxT("unknown");
}

size_t MPQFile::getPos()
{
	return pointer;
}

unsigned char* MPQFile::getBuffer()
{
	return buffer;
}

unsigned char* MPQFile::getPointer()
{
	return buffer + pointer;
}


#include <wx/tokenzr.h>

void getFileLists(std::set<FileTreeItem> &dest, bool filterfunc(wxString))
{
	for(ArchiveSet::iterator i=gOpenArchives.begin(); i!=gOpenArchives.end();++i)
	{
		HANDLE &mpq_a = *i->second;
		bool isPartial = false;
		if (i->first.AfterLast(SLASH).StartsWith(wxT("wow-update-base")))
			isPartial = false;
		else if (i->first.AfterLast(SLASH).StartsWith(wxT("wow-update-")+langName))
			isPartial = false;
		else if (i->first.AfterLast(SLASH).StartsWith(wxT("wow-update-")))
			isPartial = true;

		HANDLE fh;
		if( SFileOpenFileEx( mpq_a, "(listfile)", 0, &fh ) )
		{
			// Found!
			DWORD filesize = SFileGetFileSize( fh );
			size_t size = filesize;

			wxString temp(i->first);
			temp.MakeLower();
			int col = 0; // Black

			if ((temp.Find(wxT("wow-update-")) > -1) || (temp.Find(wxT("patch.mpq")) > -1))
				col = 1; // Blue
			else if (temp.Find(wxT("cache")) > -1 || temp.Find(wxT("patch-2.mpq")) > -1)
				col = 2; // Red
			else if(temp.Find(wxT("expansion1.mpq")) > -1 || temp.Find(wxT("expansion.mpq")) > -1)
				col = 3; // Outlands Purple
			else if (temp.Find(wxT("expansion2.mpq")) > -1 || temp.Find(wxT("lichking.mpq")) > -1)
				col = 4; // Frozen Blue
			else if (temp.Find(wxT("expansion3.mpq")) > -1)
				col = 5; // Destruction Orange
			else if (temp.Find(wxT("expansion4.mpq")) > -1 || temp.Find(wxT("patch-3.mpq")) > -1)
				col = 6; // Bamboo Green
			else if (temp.Find(wxT("alternate.mpq")) > -1)
				col = 7; // Cyan

			if (size > 0 ) {
				unsigned char *buffer = new unsigned char[size];
				SFileReadFile( fh, buffer, (DWORD)size );
				unsigned char *p = buffer, *end = buffer + size;

				while (p < end) { // if p = end here, no need to go into next loop !
					unsigned char *q=p;
					do {
						if (*q=='\r' || *q=='\n') // carriage return or new line
							break;
					} while (q++<=end);

					wxString line(reinterpret_cast<char *>(p), wxConvUTF8, q-p);
					if (line.Length()==0) 
						break;
					p = q;
					if (*p == '\r')
						p++;
					if (*p == '\n')
						p++;
					//line = line.Trim(); // delete \r\n
					if (isPartial) {
						if (line.Lower().StartsWith(wxT("base\\"))) // strip "base\\"
							line = line.Mid(5);
						else if (line.StartsWith(langName)) // strip "enus\\"
							line = line.Mid(5);
						else
							continue;
					}

					if (filterfunc(wxString(line.mb_str(), wxConvUTF8))) {
						// This is just to help cleanup Duplicates
						// Ideally I should tokenise the string and clean it up automatically
						FileTreeItem tmp;

						tmp.fileName = line;
						line.MakeLower();
						line[0] = toupper(line.GetChar(0));
						int ret = line.Find('\\');
						if (ret>-1)
							line[ret+1] = toupper(line.GetChar(ret+1));

						tmp.displayName = line;
						tmp.color = col;
						dest.insert(tmp);
					}
				}

				wxDELETEA(buffer);
				p = NULL;
				end = NULL;
			}

			SFileCloseFile( fh );
		}
	}
}
