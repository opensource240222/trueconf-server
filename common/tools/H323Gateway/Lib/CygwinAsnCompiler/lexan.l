%{
  #include <stdio.h>
  #include <stdlib.h>

  #include "frontend.h"
  #include "y.tab.h"

  FILE * out = fopen("lex.log","w");
  
  extern "C" int yywrap(void);
%}


%%
[0-9]+[^\.]{1}	{
	fprintf(out,"\n RANGE0 ");      
	char * buffer = strdup( yytext );
	unsigned int s = strlen(buffer);
	unput(buffer[s-1]);
	printf("\n\t %s char:%c",buffer,buffer[s-1]);
	yylval.srange = new VS_AsnCompilerRange;
	int dig1=0;
	unsigned int dig2=0;
	sscanf(buffer,"%d",&dig1);
	yylval.srange->low = dig1;
	yylval.srange->upp = dig1;
	yylval.srange->isExtendable = false;
	yylval.srange->rangeType = e_one_digit;
	return RANGE;
}
"TYPE-IDENTIFIER.&Type" { 
  fprintf(out,"\n TYPE-IDENTIFIER.&Type "); 
  yylval.stype = new VS_AsnSimpleTypes;
  *(yylval.stype)= e_typeid;
  return TYPE_ID; 
}
"OCTET STRING"      { 
  fprintf(out,"\n octet string "); 
  yylval.stype = new VS_AsnSimpleTypes;
  *(yylval.stype)= e_octstring;
  return OCTSTRING; 
}
"BIT STRING"      { 
  fprintf(out,"\n bit string "); 
  yylval.stype = new VS_AsnSimpleTypes;
  *(yylval.stype)= e_bitstring;
  return BITSTRING; 
}
"OBJECT IDENTIFIER" { 
  fprintf(out,"\n  object id");
  yylval.stype = new VS_AsnSimpleTypes;
  *(yylval.stype)= e_objectid;
  return OBJID; 
}
[\-]{2}[^\n]*    	        { fprintf(out,"\n -- comment2 "); yylval.stoken = 0; 
									  char * buffer = strdup( yytext );
									  unsigned size = strlen(buffer);
									  int i,j,k;
									  for(i=1;i<size-1;i++)
									  {
										if ((buffer[i]==buffer[i+1])&&(buffer[i]=='-'))
										{
											j=i+2;
											if (j<size)
											{
												//char tmp[256]={0};
												printf("\n size:%d j:%d",size,j);
												for(k=size-j-1;k>=0;k--)
												{
													//	tmp[k]=buffer[j+k];
													unput( buffer[j+k] );
													printf("\n k:%d,%c",k,buffer[j+k]);
												}
											}
										}
									  }
									  fprintf(out,"\n COMMENT:%s ",buffer);
									  printf("\n\t COMMENT:%s ",buffer);
									  free( buffer );	
									  return DASH_AND_DASH;
									}
[\(]				{ fprintf(out,"\nleft bracket ("); yylval.stoken = 0; return LB;}
[\)]				{ fprintf(out,"\nright bracket )"); yylval.stoken = 0; return RB;}
[ \t]               { fprintf(out,"\n_");              }     
[\{]				{ fprintf(out,"\nleft brace"); yylval.stoken = 0; return LBRACE;}
[\}]				{ fprintf(out,"\nright brace"); yylval.stoken = 0; return RBRACE;}
[\;]				{ fprintf(out,"\n\n ; \n");yylval.stoken = 0; return SEMICOLON;}
[\:]{2}[\=]			{ fprintf(out,"\n equal ");yylval.stoken = 0; return EQUAL;}
[\n]                { fprintf(out,"\n\n return ");     yylval.stoken = 0; return RET;}            
[\,]                { fprintf(out,"\n|,|");  yylval.stoken = 0; return COMMA;}            
[a-z]{1}[a-zA-Z0-9\-]*    { 
  char * buffer = strdup( yytext );
  yylval.sname = new VS_AsnCompilerName;
  if (0!=strncpy( yylval.sname->data, buffer , MAX_NAME_SIZE))
  { fprintf(out,"\nvarname : |%s|",buffer); 
	free( buffer );
    return VARNAME;
  }
  else 
  { fprintf(out,"\n\n Strcpy returned 0");
	free( buffer );
    exit(0);
  }
  fprintf(out,"\n\nError Unknown return! varname \n");
}
[\"]{1}[^\"]+[\"]{1}	{
char * buffer = strdup( yytext );
  yylval.sname = new VS_AsnCompilerName;
  if (strlen(buffer)>=MAX_NAME_SIZE) 
  {
	printf("\n\t Buffer to long!!!");
	exit(0);
  }
	fprintf(out,"\n\t alpabet! '%s'",buffer);  
  if (0!=memcpy(yylval.sname->data, buffer+1,strlen(buffer)-2))
  { fprintf(out,"\nvarname : |%s|",buffer); 
    return ALPHABET;
  }
  else 
  { fprintf(out,"\n\n memcpy returned 0");
    exit(0);
  }
  fprintf(out,"\n\nError Unknown return! varname \n");

}
[A-Z]+                {   
   char * token = strdup( yytext );
   fprintf(out,"\n\n special or simple:|%s|",token);     
  if (0==strcmp( "SIZE" , token ))    {yylval.stoken = 0; return SIZE;}
  if (0==strcmp( "FROM" , token ))    {yylval.stoken = 0; return FROM;}
  if (0==strcmp( "HASHED" , token ))    {yylval.stoken = 0; return HASHED;}
  if (0==strcmp( "ENCRYPTED" , token ))    {yylval.stoken = 0; return ENCRYPTED;}
  if (0==strcmp( "SIGNED" , token ))    {yylval.stoken = 0; return SIGNED;}
  if (0==strcmp( "WITH COMPONENTS" , token ))    {yylval.stoken = 0; return WITH_COMP;}
  if (0==strcmp( "OF"   , token ))    {yylval.stoken = 0; return OF;}
  if (0==strcmp( "OPTIONAL" , token ))  {yylval.stoken = 0; return OPTIONAL;}
  if (0==strcmp( "::=" , token  ))    {yylval.stoken = 0; return EQUAL;}
  if (0==strcmp( "SET" , token  ))  
  { 
    yylval.stype = new VS_AsnSimpleTypes ;
    *yylval.stype = e_set;
    return SET;
  }   
  
  if (0==strcmp( "SEQUENCE" , token ))  
  { 
    yylval.stype = new VS_AsnSimpleTypes ;
    *(yylval.stype) = e_sequence;
    return SEQUENCE;
  }   
  if (0==strcmp( "CHOICE" , token ))    
  {
    yylval.stype = new VS_AsnSimpleTypes ;
    *(yylval.stype) = e_choice;
    fprintf(out,"\n!choice in!");
    return CHOICE;
  }   
  if (0==strcmp( "BOOLEAN" , token  ))    
  {
    yylval.stype = new VS_AsnSimpleTypes ;
    *(yylval.stype) = e_bool;
    return BOOL;
  }   
  if (0==strcmp( "NULL" , token ))    
  {
    yylval.stype = new VS_AsnSimpleTypes ;
    *yylval.stype = e_null;
    return NULLER;
  }   
  if (0==strcmp( "ENUMERATED" , token ))    
  {
    yylval.stype = new VS_AsnSimpleTypes ;
    *yylval.stype = e_null;
    return ENUM;
  }
  if (0==strcmp( "INTEGER" , token ))    
  {
    yylval.stype = new VS_AsnSimpleTypes ;
    *yylval.stype = e_integer;
    return INTEGER;
  }
  fprintf(out,"\n \n\n !!! [A-Z]+ Unknown WORD : %s ",token);      
}   
[A-Z][a-zA-Z0-9\-]*      { 
  fprintf(out,"\n typename");          
  char * token = strdup( yytext );
  if (0==strcmp( "IA5String" , token  ))    
  {
    yylval.stype = new VS_AsnSimpleTypes ;
    *(yylval.stype) = e_ia5string;
    return IA5STRING;
  }   
  if (0==strcmp( "BMPString" , token  ))    
  {
    yylval.stype = new VS_AsnSimpleTypes ;
    *(yylval.stype) = e_bmpstring;
    return BMPSTRING;
  }
  if (0==strcmp( "NumericString" , token  ))    
  {
    yylval.stype = new VS_AsnSimpleTypes ;
    *(yylval.stype) = e_numstring;
    return NUMSTRING;
  }
  if (0 == strcmp("GeneralString", token))
  {
      yylval.stype = new VS_AsnSimpleTypes;
      *yylval.stype = e_genstring;
      return GENSTRING;
  }

  yylval.sname = new VS_AsnCompilerName;
  if ( 0!=strncpy( yylval.sname->data , token ,MAX_NAME_SIZE))
	return TYPENAME;
  fprintf(out,"\n\n Error! End of [A-Z][a-zA-Z-0-9]* ");   
}
[\.]{3}               { fprintf(out,"\n DOTS"); {yylval.stoken = 0; return DOTS;}         }
[\-]?[0-9]+[\.]{2}[0-9]+     { 
  fprintf(out,"\n RANGE ");      
  char * buffer = strdup( yytext );
  yylval.srange = new VS_AsnCompilerRange;
  int dig1=0;
  unsigned int dig2=0;
  //printf("\n\n\t*%s*\n\n",buffer);
  sscanf(buffer,"%d..%lu",&dig1,&dig2);
  yylval.srange->low = dig1;
  yylval.srange->upp = dig2;
  yylval.srange->isExtendable = false;
  yylval.srange->rangeType = e_range;
  fprintf(out,"\n : (%d..%d)",dig1,dig2);
  return RANGE;
}
%%

int yywrap(void)
{
	return -1;
}

