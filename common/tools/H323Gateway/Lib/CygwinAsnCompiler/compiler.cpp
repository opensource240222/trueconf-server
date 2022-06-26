#include <stdlib.h>
#include <stdio.h>

#include "frontend.h"

char header[]="					\
\n#pragma once					\
\n								\
\n#include <Windows.h>			\
\n								\
\n#include \"VS_Containers.h\"		\
\n#include \"VS_AsnBuffers.h\"		\
\n#include \"VS_CommonMessages.h\"	\
\n\n";

int main(int argc,char ** argv)
{
	FILE * config;
	config = fopen("compiler.cfg","r");
	char tmp[MAX_NAME_SIZE];
	if (config!=NULL)
	{	unsigned int i=0;
		i = fread( tmp,sizeof(char),MAX_NAME_SIZE - 1, config);
		if (i>=1)
		{
			strncpy( prefix , tmp , i);
			prefix[i] = 0;
		}
	} else printf("\n\t NO CONFIG FILE. USE DEFAULTS.");

	if (argc>1)
		yyin = fopen(argv[1],"r");
	else yyin = stdin;
	printf("\n Start!");
	yyparse();

	//unsigned d;
	 long ii;
	VS_AsnCompilerBuffer buffer;
	VS_AsnCompilerBuffer buffer_cpp;
	FILE * itog= fopen("itog.h","w");
	FILE * itog_c= fopen("itog.cpp","w");
	buffer.AddString("\n");
	buffer_cpp.AddString("\n");

	printf("\n\t Value of world.meta_type_index:%lu",world.meta_type_index);

	//char chbuf[1000];
	buffer.AddString(header,strlen(header)+1);
	//for(ii=0;ii<world.meta_type_index;ii++)
	//{
	//	printf("\n Meta Short Key: %d",world.meta_type_storage[ii]);
	//	world.class_tree[ world.meta_type_storage[ii] ].MakeShortWalk( world , buffer , buffer_cpp );
	//}

	for(ii=world.meta_type_index - 1;ii>=0;ii--)
	{
		printf("\n Meta Key: %d",world.meta_type_storage[ii]);
		world.class_tree[ world.meta_type_storage[ii] ].MakeWalk( world , buffer , buffer_cpp );
	}

	printf("\nI`m exit ");
	buffer.SaveToFile( itog );
	fclose( itog );
	buffer_cpp.SaveToFile( itog_c );
	fclose( itog_c );




  return 0;
}
