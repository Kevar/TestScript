#pragma once

#include "TextData.h"

//----------------------------------------
struct FileData
{
	const char* Path{ nullptr };
	TextData Text;
};

bool LoadFile( const char* path, FileData& out_file_data );
void CloseFile( FileData& file_data );