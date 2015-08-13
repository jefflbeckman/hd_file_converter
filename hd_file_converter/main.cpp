#include <iostream>
#include <fstream>
#include <string.h>



void print_usage()
{
	std::cout << "Please give at least one argument with a valid filepath.\n" 
			  << "Each filename should be in the format MDDCCHHH.SCH"
			  << std::endl;
}

bool validate_filename(char* path_buffer)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	_splitpath_s(path_buffer, drive, dir, fname, ext);
	for(int i = 0; ext[i]; i++){
		ext[i] = tolower(ext[i]);
	}

	return (strcmp(ext, ".sch") == 0 && strlen(fname) == 8 && strspn(fname, "0123456789") == strlen(fname));
}

void modify_channelID_in_filename(char* pathdest, char* pathsrc)
{
	//shameless taken from msdn example https://msdn.microsoft.com/en-us/library/sh8yw82b%28v=vs.71%29.aspx
	char path_buffer[_MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	_splitpath(pathsrc, drive, dir, fname, ext);

	int HDchannel = 100 - ((atoi(fname) / 1000) % 100);
	/* lexical shifting of digits then manually put them in place in the filename*/
	fname[3] = ((HDchannel / 10) % 10) + '0';
	fname[4] = ((HDchannel / 1) % 10) + '0';
	
	_makepath(pathdest, drive, dir, fname, ext);
}

void backup_filename(char* pathdest, char* pathsrc)
{
	//shameless taken from msdn example https://msdn.microsoft.com/en-us/library/sh8yw82b%28v=vs.71%29.aspx
	char path_buffer[_MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	_splitpath(pathsrc, drive, dir, fname, ext);

	int HDchannel = 100 - ((atoi(fname) / 1000) % 100);
	/* lexical shifting of digits then manually put them in place in the filename*/
	fname[3] = ((HDchannel / 10) % 10) + '0';
	fname[4] = ((HDchannel / 1) % 10) + '0';

	_makepath(pathdest, drive, dir, fname, ext);
}

int main(int argc, char* argv[])
{

	int pause; // input to keep window open
	int index = 0;
	int changed_record_count = 0;
	int changed_file_count = 0;
	FILE* in_file;
	FILE* out_file;

#ifdef _WIN32
	system("cls");
#endif


	if (argc < 2)
	{
		print_usage();
		exit(1);
	}

	/* Validate arguments before making any changes*/
	while (++index < argc) //ignore first argument (program name)
	{
		if (!validate_filename(argv[index]))
		{
			std::cout << argv[index]  << " is an invalid argument" << std::endl;
			print_usage();
			break;
		}
	}


	/* This is the work loop that rewrites each file*/
	index = 0;
	while (++index < argc)
	{
		char filepath[_MAX_PATH];
		char new_filepath[_MAX_PATH];
		char* buffer;
		int flen;
		int ret;

		strncpy(filepath, argv[index], _MAX_PATH);
		in_file = fopen(filepath, "rb");
		if (in_file == NULL)
		{
			std::cout << "Error opening file:" << filepath << " errno: " << errno <<std::endl;
			std::cout << "skipping this file and continuing" << std::endl;
			continue;
		}

		/* find the total size of the file*/
		fseek(in_file, 0, SEEK_END);
		flen = ftell(in_file);
		rewind(in_file);

		/* read the file into memory*/
		buffer = (char*)malloc(sizeof(char) * flen + 1);
		memset(buffer, 0, sizeof(char) * flen + 1);
		ret = fread(buffer, 1, flen, in_file);
		if (ret != flen) 
		{
			std::cout <<  "Error reading file:" << filepath << std::endl;
			free(buffer);
			fclose(in_file);
			continue;
		}
		
		/* tokenize by line, modify each line in our buffer*/
		int line_no = 0;
		changed_record_count = 0;
		char * token = strtok(buffer,"\r\n");
		while (token != NULL)
		{
			line_no++;
			if (token[62] == '0' && token[63] == '0')
			{
				token[62] = 'H';
				token[63] = '_';
				changed_record_count++;
			}
			else
			{
				std::cout << "potential incorrect record found in line " << line_no << " in file " << filepath << std::endl;
			}

			token = strtok(NULL, "\r\n");
		}
		
		fclose(in_file);

		//write to new filename
		modify_channelID_in_filename(new_filepath, filepath);

		std::cout << "Found file: " << filepath << "\nModifying and writing " << changed_record_count << " records to: " << new_filepath << std::endl;
	
		out_file = fopen(new_filepath, "wb");
		ret = fwrite(buffer, 1, flen, out_file);
		fclose(out_file);
		free(buffer);
	}



#ifdef _WIN32
	system("pause");
#endif
}