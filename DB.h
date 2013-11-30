#ifndef _DB_H
#define _DB_H_

#include <cassert>
#include <string>

#include <wx/wx.h>

class DBCFile
{
public:
	DBCFile(const wxString &filename);
	~DBCFile();

	// Open database. It must be openened before it can be used.
	bool open();

	// TODO: Add a close function?

	// Database exceptions
	class Exception
	{
	public:
		Exception(const wxString &message): message(message)
		{ }
		virtual ~Exception()
		{ }
		const wxString &getMessage() {return message;}
	private:
		wxString message;
	};

	// 
	class NotFound: public Exception
	{
	public:
		NotFound(): Exception(wxT("Key was not found"))
		{ }
	};

	// Iteration over database
	class Iterator;
	class Record
	{
	public:
		Record& operator= (const Record& r)
		{
			file = r.file;
			offset = r.offset;
			return *this;
		}
		float getFloat(size_t field) const
		{
			assert(field < file.fieldCount);
			return *reinterpret_cast<float*>(offset+field*4);
		}
		unsigned int getUInt(size_t field) const
		{
			assert(field < file.fieldCount);
			return *reinterpret_cast<unsigned int*>(offset+(field*4));
		}
		int getInt(size_t field) const
		{
			assert(field < file.fieldCount);
			return *reinterpret_cast<int*>(offset+field*4);
		}
		unsigned char getByte(size_t ofs) const
		{
			assert(ofs < file.recordSize);
			return *reinterpret_cast<unsigned char*>(offset+ofs);
		}
		wxString getString(size_t field) const
		{
			assert(field < file.fieldCount);
			size_t stringOffset = getUInt(field);
			if (stringOffset >= file.stringSize)
				stringOffset = 0;
			assert(stringOffset < file.stringSize);
			//char * tmp = (char*)file.stringTable + stringOffset;
			//unsigned char * tmp2 = file.stringTable + stringOffset;
			return wxString(reinterpret_cast<char*>(file.stringTable + stringOffset), wxConvUTF8);
		}
	private:
		DBCFile &file;
		unsigned char *offset;
		Record(DBCFile &file, unsigned char *offset): file(file), offset(offset) {}

		friend class DBCFile;
		friend class Iterator;
	};

	/* Iterator that iterates over records */
	class Iterator
	{
	public:
		Iterator(DBCFile &file, unsigned char *offset): 
		  record(file, offset) {}
		  /// Advance (prefix only)
		  Iterator & operator++() { 
			  record.offset += record.file.recordSize;
			  return *this; 
		  }	
		  /// Return address of current instance
		  Record const & operator*() const { return record; }
		  const Record* operator->() const {
			  return &record;
		  }
		  /// Comparison
		  bool operator==(const Iterator &b) const
		  {
			  return record.offset == b.record.offset;
		  }
		  bool operator!=(const Iterator &b) const
		  {
			  return record.offset != b.record.offset;
		  }
	private:
		Record record;
	};

	// Get record by id
	Record getRecord(size_t id);
	/// Get begin iterator over records
	Iterator begin();
	/// Get begin iterator over records
	Iterator end();
	/// Trivial
	size_t getRecordCount() const { return recordCount; }
	size_t getFieldCount() const { return fieldCount; }
	wxString getFilename() { return filename; }
	size_t size() const { return recordCount; }

private:
	wxString filename;
	size_t recordSize;
	size_t recordCount;
	size_t fieldCount;
	size_t stringSize;
	unsigned char *data;
	unsigned char *stringTable;
};


class ItemDisplayDB: public DBCFile
{
public:
	ItemDisplayDB(): DBCFile(wxT("DBFilesClient\\ItemDisplayInfo.dbc")) {}
	~ItemDisplayDB() {}

	/// Fields
	static const size_t ItemDisplayID = 0;	// uint
	static const size_t Model = 1;			// string, modelleft
	static const size_t Model2 = 2;			// string, modelright
	static const size_t Skin = 3;			// string, textureleft
	static const size_t Skin2 = 4;			// string, textureright
	//static const size_t Icon = 5;			// string
	//static const size_t Texture = 6;			// string
	static const size_t GloveGeosetFlags = 7;		// uint, (0,1,2,3,4,5)
	static const size_t BracerGeosetFlags = 8;		// uint, (0,1,2,3)
	static const size_t RobeGeosetFlags = 9;		// uint, (0,1)
	static const size_t BootsGeosetFlags = 10;		// uint, (0,1,2,4,6)
	//static const size_t Unknown = 11;		// uint
	//static const size_t ItemGroupSounds = 12;			// uint
	//static const size_t GeosetVisID1 = 13;	// uint, HelmetGeosetVisData.dbc
	//static const size_t GeosetVisID2 = 14;	// uint, HelmetGeosetVisData.dbc
	static const size_t TexArmUpper = 15;	// string
	static const size_t TexArmLower = 16;	// string
	static const size_t TexHands = 17;		// string
	static const size_t TexChestUpper = 18;	// string
	static const size_t TexChestLower = 19;	// string
	static const size_t TexLegUpper = 20;	// string
	static const size_t TexLegLower = 21;	// string
	static const size_t TexFeet = 22;		// string
	static const size_t Visuals = 23;		// uint
	//static const size_t ParticleColor = 24;	// uint

	Record getById(unsigned int id);
	bool hasId(unsigned int id);

private:

};

//1月16日 加入 CHarSectionDB
// As specified in http://www.madx.dk/wowdev/wiki/index.php?title=CharSections.dbc
class CharSectionsDB: public DBCFile
{
public:
	CharSectionsDB(): DBCFile(wxT("DBFilesClient\\CharSections.dbc")) {}
	~CharSectionsDB() {}

	/// Fields
	static const size_t SectonID = 0;	// uint
	static const size_t Race = 1;		// uint
	static const size_t Gender = 2;		// uint
	static const size_t Type = 3;		// uint
//摘取的是 defind了WotLK的那段
	static const size_t Tex1 = 4;		// string
	static const size_t Tex2 = 5;		// string
	static const size_t Tex3 = 6;		// string
	static const size_t IsNPC = 7;		// Flags, uint, IsNPC = 0x1 ?, IsDeathKnight?
	static const size_t Section = 8;	// uint
	static const size_t Color = 9;		// uint

	/// Types
	enum{
		SkinType = 0,
		FaceType,
		FacialHairType,
		HairType,
		UnderwearType
	};
	/*
	static const size_t SkinType = 0;
	static const size_t FaceType = 1;
	static const size_t FacialHairType = 2;
	static const size_t HairType = 3;
	static const size_t UnderwearType = 4;*/

	Record getByParams(size_t race, size_t gender, size_t type, size_t section, size_t color, size_t npc);
	int getColorsFor(size_t race, size_t gender, size_t type, size_t section, size_t npc);
	int getSectionsFor(size_t race, size_t gender, size_t type, size_t color, size_t npc);
};

//這個是 一個animation的id 與其 動畫的名稱對應的DB 比如說 13-> Attack2H等
class AnimDB: public DBCFile
{
public:
	AnimDB(): DBCFile(wxT("DBFilesClient\\AnimationData.dbc")) {}
	~AnimDB() {}

	/// Fields
	static const size_t AnimID = 0;		// uint
	static const size_t Name = 1;		// string
	// static const size_t WeaponState = 2;	// int, 32 = pull weapons out during animation. 16 and 4 weapons are put back.
	// static const size_t Flags = 3;
	// static const size_t Unkonwn = 4;
	// static const size_t Preceeding; // The animation, preceeding this one.
	// static const size_t RealId; // Same as ID for normal animations. (WotLK)
	// static const size_t Group; // 0 for normal, 3 for fly. (WotLK)

	Record getByAnimID(unsigned int id);
};




#endif