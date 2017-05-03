#include "Pch.h"
#include "Library.h"


eastl::vector<uint8_t> LoadFile(const char* FileName)
{
	FILE* File = fopen(FileName, "rb");
	assert(File);
	fseek(File, 0, SEEK_END);
	long Size = ftell(File);
	assert(Size != -1);
	eastl::vector<uint8_t> Content(Size);
	fseek(File, 0, SEEK_SET);
	fread(&Content[0], 1, Content.size(), File);
	fclose(File);
	return Content;
}
