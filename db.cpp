#include "DB.h"
#include "mpq.h"
DBCFile::DBCFile(const wxString &filename) : filename(filename)
{
	data = NULL;
	stringTable = NULL;
	recordSize = 0;
	recordCount = 0;
	fieldCount = 0;
	stringSize = 0;
}

bool DBCFile::open()
{
	enum FileType {
		FT_UNK,
		FT_WDBC,
		FT_WDB2,
	};
	int db_type = FT_UNK;
	//if (filename.Lower().EndsWith(wxT("item.dbc")) && gameVersion >= VERSION_CATACLYSM) {
	if (filename.Lower().EndsWith(wxT("item.dbc")) ){
		filename = filename.BeforeLast('.') + wxT(".db2");
	}

	//g_modelViewer->SetStatusText(wxT("Initiating ")+filename+wxT(" Database..."));
	MPQFile f(filename);
	// Need some error checking, otherwise an unhandled exception error occurs
	// if people screw with the data path.
	if (f.isEof())
		return false;

	char header[5];
	unsigned int na,nb,es,ss;

	f.read(header, 4); // File Header

	if (strncmp(header, "WDBC", 4) == 0)
		db_type = FT_WDBC;
	else if (strncmp(header, "WDB2", 4) == 0)
		db_type = FT_WDB2;

	if (db_type == FT_UNK) {
		f.close();
		data = NULL;
		wxLogMessage(wxT("Critical Error: An error occured while trying to read the DBCFile %s."), filename.c_str());
		return false;
	}

	//assert(header[0]=='W' && header[1]=='D' && header[2]=='B' && header[3] == 'C');

	f.read(&na,4); // Number of records
	f.read(&nb,4); // Number of fields
	f.read(&es,4); // Size of a record
	f.read(&ss,4); // String size

	if (db_type == FT_WDB2) {
		f.seekRelative(28);
		// just some buggy check
		unsigned int check;
		f.read(&check, 4);
		if (check == 6) // wrong place
			f.seekRelative(-20);
		else // check == 17, right place
			f.seekRelative(-4);
	}

	recordSize = es;
	recordCount = na;
	fieldCount = nb;
	stringSize = ss;
	//assert(fieldCount*4 == recordSize);
	// not always true, but it works fine till now
	assert(fieldCount*4 >= recordSize);

	data = new unsigned char[recordSize*recordCount+stringSize];
	stringTable = data + recordSize*recordCount;
	if (db_type == FT_WDB2) {
		f.seek(f.getSize() - recordSize*recordCount - stringSize);
	}
	f.read(data, recordSize*recordCount+stringSize);
	f.close();
	return true;
}

DBCFile::~DBCFile()
{
	delete [] data;
}

DBCFile::Record DBCFile::getRecord(size_t id)
{
	//assert(data);
	return Record(*this, data + id*recordSize);
}

DBCFile::Iterator DBCFile::begin()
{
	//assert(data);
	return Iterator(*this, data);
}
DBCFile::Iterator DBCFile::end()
{
	//assert(data);
	return Iterator(*this, stringTable);
}

ItemDisplayDB::Record ItemDisplayDB::getById(unsigned int id)
{
	for(Iterator i=begin(); i!=end(); ++i)
	{
		//if(i->getUInt(ItemDisplayID) < 70000)
			//printf("%d\n",i->getUInt(ItemDisplayID));
		if (i->getUInt(ItemDisplayID)==id)
			return (*i);
	}
	//wxLogMessage(wxT("NotFound: %s:%s#%d"), __FILE__, __FUNCTION__, __LINE__);
	throw NotFound();
}

// Sections

int CharSectionsDB::getColorsFor(size_t race, size_t gender, size_t type, size_t section, size_t npc)
{
	int n = 0;
	for(Iterator i=begin(); i!=end(); ++i)
	{
		// don't allow NPC skins ;(
		//if (gameVersion < VERSION_WOTLK) {
			//if (i->getUInt(Race)==race && i->getUInt(Gender)==gender && i->getUInt(Type)==type && i->getUInt(Section)==section && i->getUInt(IsNPC)==npc) {
				//n++;
			//}
		//} else {
		if (i->getUInt(Race)==race && i->getUInt(Gender)==gender && i->getUInt(Type)==type && i->getUInt(Section)==section) {
			n++;
		}
		//}
	}

	return n;
}

int CharSectionsDB::getSectionsFor(size_t race, size_t gender, size_t type, size_t color, size_t npc)
{
	int n = 0;
	for(Iterator i=begin(); i!=end(); ++i)
	{
		///if (gameVersion < VERSION_WOTLK) {
			//if (i->getUInt(Race)==race && i->getUInt(Gender)==gender && i->getUInt(Type)==type && i->getUInt(Color)==color && i->getUInt(IsNPC)==npc) {
			//	n++;
			//}
		//} else {
		if (i->getUInt(Race)==race && i->getUInt(Gender)==gender && i->getUInt(Type)==type && i->getUInt(Color)==color) {
			n++;
		}
		//}
	}
	return n;
}

CharSectionsDB::Record CharSectionsDB::getByParams(size_t race, size_t gender, size_t type, size_t section, size_t color, size_t npc)
{
	for(Iterator i=begin(); i!=end(); ++i)
	{
		//if (gameVersion < VERSION_WOTLK) {
			//if (i->getUInt(Race)==race && i->getUInt(Gender)==gender && i->getUInt(Type)==type && i->getUInt(Section)==section && i->getUInt(Color)==color && i->getUInt(IsNPC)==npc)
				//return (*i);
		//} else {
		if (i->getUInt(Race)==race && i->getUInt(Gender)==gender && i->getUInt(Type)==type && i->getUInt(Section)==section && i->getUInt(Color)==color)
			return (*i);
		//}
	}
	//wxLogMessage(wxT("NotFound: %s:%s#%d race:%d, gender:%d, type:%d, section:%d, color:%d"), __FILE__, __FUNCTION__, __LINE__, race, gender, type, section, color);
	throw NotFound();
}


AnimDB::Record AnimDB::getByAnimID(unsigned int id)
{
	/// Brute force search for now
	for(Iterator i=begin(); i!=end(); ++i)
	{
		if(i->getUInt(AnimID) == id)
			return (*i);
	}
	//wxLogMessage(wxT("NotFound: %s:%s#%d"), __FILE__, __FUNCTION__, __LINE__);
	throw NotFound();
}
