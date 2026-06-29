#include "FileReader.h"

#include <stdio.h>

//----------------------------------------
bool LoadFile( const char* path, FileData& out_file_data )
{
	bool success = false;

	out_file_data.Path = path;

	FILE* file;
	errno_t error = fopen_s( &file, out_file_data.Path, "r" );	// #TODO : tenter en "rb" et pas "r" pour retrouver les \r

	if( file != nullptr && error == 0 )
	{
		if( fseek( file, 0, SEEK_END ) == 0 )
		{
			long size = ftell( file );	// Note : ça semble être une taille maximale mais les données lues sont souvent plus courtes d'1 ou 2 caractères. Pas encore compris pourquoi. Addendum : c'est probablement à cause du stripping des caractères \r
			if( size != -1 )
			{
				out_file_data.Text.Buffer = new char[size];

				if( fseek( file, 0, SEEK_SET ) == 0 )
				{
					out_file_data.Text.Length = fread( (void*)out_file_data.Text.Buffer, sizeof( char ), size, file );

					if( ferror( file ) == 0 )
					{
						success = true;
					}
				}
			}
		}

		fclose( file );
	}

	if( !success )
	{
		CloseFile( out_file_data );
	}

	return success;
}

//----------------------------------------
void CloseFile( FileData& file_data )
{
	delete[] file_data.Text.Buffer;
	file_data.Text.Buffer = nullptr;
	file_data.Text.Length = 0;
}