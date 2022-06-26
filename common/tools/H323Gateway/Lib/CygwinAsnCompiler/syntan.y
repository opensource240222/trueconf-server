%{
#include "frontend.h"
bool isNameIsAlphabet=false;
bool isPostfixMeta = true;
bool isTypeFilled = false;
VS_AsnCompilerContainer world;
VS_AsnSimpleTypes	current_type;
VS_AsnCompilerName  current_name;
VS_AsnTypeCompilerContainer current_meta_type;

bool isOptional;/// No realy needs in that parametr. =)) It is atavism.

int yyerror(const char*  s);
%}

%union{
  VS_AsnCompilerName        * sname;
  VS_AsnSimpleTypes         * stype;
  int                         stoken;
  VS_AsnCompilerRange       * srange;
};

%error-verbose

%token <srange> RANGE
%token <sname>  VARNAME TYPENAME ALPHABET
%token <stype>  INTEGER BOOL NULLER ENUM OBJID BITSTRING OCTSTRING NUMSTRING IA5STRING PRISTRING GENSTRING BMPSTRING CHOICE SEQUENCE SET SIGNED ENCRYPTED HASHED TYPE_ID
%token <stoken> OPTIONAL DOTS ARRAY COMMA EQUAL OF SIZE RET SEMICOLON LBRACE RBRACE LB RB FROM DASH_AND_DASH WITH_COMP
%%

world		: type									{	printf("\n\t!WORLD BEGIN");	int d;/*scanf("%d",&d);*/		}
			| world type							{	printf("\n\t!WORLD CONTINUE");int d;/*scanf("%d",&d);*/			}
			;
type        : metatypepref classbody RBRACE			{	world.MetaOut();printf("\n\t!exit from META"); isPostfixMeta = true;}
			| TYPENAME EQUAL array SIZE LB RANGE RB OF TYPENAME {	
					printf("\n\t 1 IN SIMPLE META : type = %d type2 = %d",current_type,current_meta_type.type);
					if (current_type==e_sequence) current_type = e_sequence_of;
					if (current_type==e_set) current_type = e_set_of;
					
						isTypeFilled = true;
						VS_AsnCompilerRange rangess;
						VS_AsnSimpleTypes   typess = e_undefined;
						isNameIsAlphabet = false;
					
						current_meta_type.type = e_undefined;
						current_meta_type.isTypeSizeExist = false;
						current_meta_type.arrayType = current_type;
						current_meta_type.typeRange = rangess;
						current_meta_type.arrayRange = rangess;
						current_meta_type.typeName = *($9);

					current_meta_type.arrayRange = *($6);
					current_meta_type.isArraySizeExist = true;
					current_meta_type.isArrayExist = true;
					isNameIsAlphabet = false;
					isTypeFilled = true;
					world.AddMetaType(*($1) , current_type,isNameIsAlphabet,current_name,isTypeFilled,current_meta_type);
					world.MetaOut();
					isTypeFilled = false;
					isNameIsAlphabet = false;
					printf("\n\t I`v found a simple META!!!");	
				}
			| TYPENAME EQUAL postfix				{	
					if (current_type==e_sequence) current_type = e_sequence_of;
					if (current_type==e_set) current_type = e_set_of;				
					//current_meta_type.type = current_type;
					world.AddMetaType(*($1) , current_type,isNameIsAlphabet,current_name,isTypeFilled,current_meta_type);
					printf("\n\t 2 IN SIMPLE META : type = %d",current_type);
					world.MetaOut();
					isTypeFilled = false;
					isNameIsAlphabet = false;
					printf("\n\t I`v found a simple META!!!");	
				}
			| type DASH_AND_DASH					{   printf("\n\t!META DASHES RET");			int d;	/*scanf("%d",&d);*/	}	
			| type RET								{   printf("\n\t!META RET");			int d;	/*scanf("%d",&d);*/	}	
            ;
embededtype : embdtypepref classbody RBRACE         {	world.EmbededOut( );	isOptional = false;	printf("\n\tembeded out!!!");}
            ;
classbody   : RET									{									printf("\n\tclassbody| ret");}
			| lines									{									printf("\n\tclassbody| lines");}
			| embededtype							{	isOptional = false;				printf("\n\tclassbody| emb");}
			| embededtype OPTIONAL					{	world.LastEmbeddedIsOptional( true );	printf("\n\tclassbody| emb opt");}
			| classbody lines						{									printf("\n\tclassbody  lines");}
			| classbody embededtype					{	isOptional = false;				printf("\n\tclassbody  emb");}
			| classbody embededtype OPTIONAL		{	world.LastEmbeddedIsOptional( true );	printf("\n\tclassbody  emb opt");}
			| classbody COMMA						{									printf("\n\tclassbody  ,");}
			| classbody DASH_AND_DASH    			{									printf("\n\tclassbody  -- DASH_AND_DASH");}
			| classbody RET							{									printf("\n\tclassbody  ret");}
			;
metatypepref: TYPENAME EQUAL container RET LBRACE		{
					VS_AsnCompilerName namess;
					
					world.AddMetaType(*($1) , current_type,false,namess,false,current_meta_type );
						printf("\n\tMetatype container");
				}
			| TYPENAME EQUAL container DASH_AND_DASH RET LBRACE		{
					VS_AsnCompilerName namess;
					
					world.AddMetaType(*($1) , current_type,false,namess,false,current_meta_type );
						printf("\n\tMetatype container");
				}	
			;
lines       : prefix postfix							{world.NextLine();	printf("\n\tline prefix postfix \n");}
            | DOTS										{world.AddDots();	printf("\n\tline DOTS");}            
            ;
postfix     : TYPENAME									{
					printf("\n\tpostfix 1");
					VS_AsnCompilerRange rangess;
					VS_AsnSimpleTypes   typess = e_undefined;
					isNameIsAlphabet = false;
					if (!isPostfixMeta) world.AddLinePostfix(*($1), false ,typess ,false,rangess,false );
					else 
					{	isTypeFilled = true;
						VS_AsnCompilerRange rangess;
						current_type = e_undefined;
						current_meta_type.type = e_undefined;
						current_meta_type.isTypeSizeExist = false;
						current_meta_type.arrayType = e_undefined;
						current_meta_type.typeRange = rangess;
						current_meta_type.arrayRange = rangess;
						current_meta_type.isArraySizeExist = false;
						current_meta_type.isArrayExist = false;
						current_meta_type.typeName = *($1);
					}
														}
            | TYPENAME LB RANGE RB						{
					printf("\n\tpostfix 2");
					isNameIsAlphabet = false;
					VS_AsnSimpleTypes   typess = e_undefined;
					if (!isPostfixMeta) world.AddLinePostfix(*($1), false ,typess ,false, *($3) ,false );
					else 
					{	isTypeFilled = true;
						VS_AsnCompilerRange rangess;
						current_type = e_undefined;
						current_meta_type.type = e_undefined;
						current_meta_type.isTypeSizeExist = true;
						current_meta_type.arrayType = e_undefined;
						current_meta_type.typeRange = *($3);
						current_meta_type.arrayRange = rangess;
						current_meta_type.isArraySizeExist = false;
						current_meta_type.isArrayExist = false;
						current_meta_type.typeName = *($1);
					}
														}
            | TYPENAME OPTIONAL							{
					VS_AsnCompilerRange rangess;
					isNameIsAlphabet = false;
					VS_AsnSimpleTypes   typess = e_undefined;
					if (!isPostfixMeta) world.AddLinePostfix(*($1), false ,typess ,false,rangess,true );	
					else 
					{
						printf("\n\t SYNTAXIs ERROR i n postfix3!!!!");
						exit(0);
					}					
					printf("\n\tpostfix 3");
														}
            | TYPENAME LB RANGE RB OPTIONAL				{
					printf("\n\tpostfix 4");
					isNameIsAlphabet = false;
					VS_AsnSimpleTypes   typess = e_undefined;
					if (!isPostfixMeta)  world.AddLinePostfix(*($1), false ,typess ,false,*($3),true );	
					else 
					{
						printf("\n\t SYNTAXIs ERROR i n postfix4!!!!");
						exit(0);
					}									
														}
            | simple									{
					printf("\n\t postfix simple5");
					VS_AsnCompilerName namess;
					VS_AsnCompilerRange rangess ;
					isNameIsAlphabet = false;
					if (!isPostfixMeta)  world.AddLinePostfix( namess , true , current_type ,false,rangess,false );
					else 
					{	isTypeFilled = true;
						VS_AsnCompilerRange rangess;
						//current_type = *($1);
						current_meta_type.type = current_type;
						current_meta_type.isTypeSizeExist = false;
						current_meta_type.arrayType = e_undefined;
						current_meta_type.typeRange = rangess;
						current_meta_type.arrayRange = rangess;
						current_meta_type.isArraySizeExist = false;
						current_meta_type.isArrayExist = false;
						current_meta_type.typeName = namess;
					}
														}
            | simple LB RANGE RB						{ 
					printf("\n\tpostfix 6");
					isNameIsAlphabet = false;
					VS_AsnCompilerName namess;
					if (!isPostfixMeta) world.AddLinePostfix( namess , true , current_type ,true, *($3) ,false );///3?
					else 
					{	isTypeFilled = true;
						VS_AsnCompilerRange rangess;
						//current_type =  *($1);
						current_meta_type.type = current_type;
						current_meta_type.isTypeSizeExist = true;
						current_meta_type.arrayType = e_undefined;
						current_meta_type.typeRange = *($3);
						current_meta_type.arrayRange = rangess;
						current_meta_type.isArraySizeExist = false;
						current_meta_type.isArrayExist = false;
						current_meta_type.typeName = namess;
					}					
														}
            | simple OPTIONAL							{
					printf("\n\tpostfix 7");
					isNameIsAlphabet = false;
					VS_AsnCompilerName namess;
					VS_AsnCompilerRange rangess;
					if (!isPostfixMeta) world.AddLinePostfix( namess , true , current_type ,false,rangess,true );
					else 
					{
						printf("\n\t SYNTAXIs ERROR i n postfix7!!!!");
						exit(0);
					}						
														}
            | simple LB RANGE RB OPTIONAL				{ 
					printf("\n\tpostfix 8");
					VS_AsnCompilerName namess;
					isNameIsAlphabet = false;
					if (!isPostfixMeta) world.AddLinePostfix( namess , true , current_type ,true,*($3), true );
					else 
					{
						printf("\n\t SYNTAXIs ERROR i n postfix8!!!!");
						exit(0);
					}						
														}
            | simple LB SIZE LB RANGE RB RB				{ 
					printf("\n\tpostfix 9");
					VS_AsnCompilerName namess;
					isNameIsAlphabet = false;
					if (!isPostfixMeta) world.AddLinePostfix( namess , true , current_type ,true,*($5), false );///5?					
					else 
					{	isTypeFilled = true;
						VS_AsnCompilerRange rangess;
						//current_type = *($1);
						current_meta_type.type = current_type;
						current_meta_type.isTypeSizeExist = true;
						current_meta_type.arrayType = e_undefined;
						current_meta_type.typeRange = *($5);
						current_meta_type.arrayRange = rangess;
						current_meta_type.isArraySizeExist = false;
						current_meta_type.isArrayExist = false;
						current_meta_type.typeName = namess;
					}						
														}
            | simple LB SIZE LB RANGE RB RB OPTIONAL	{ 
					printf("\n\tpostfix 10");
					VS_AsnCompilerName namess;
					isNameIsAlphabet = false;
					if (!isPostfixMeta) world.AddLinePostfix( namess , true , current_type ,true, *($5) , true );///5?					
					else 
					{
						printf("\n\t SYNTAXIs ERROR i n postfix10!!!!");
						exit(0);
					}						
														}
			| simple LB FROM LB ALPHABET RB RB { 
					printf("\n\tpostfix 11");
					isNameIsAlphabet = true;
					current_name = *($5);
					if (!isPostfixMeta) 					
					{
						VS_AsnCompilerRange rangess;
						world.AddAlphabetLinePostfix( *($5), true , current_type ,false, rangess , false );///5?					
					}
					else 
					{	VS_AsnCompilerName namess;
						isTypeFilled = true;
						VS_AsnCompilerRange rangess;
						//current_type = *($1);
						current_meta_type.type = current_type;
						current_meta_type.isTypeSizeExist = false;
						current_meta_type.arrayType = e_undefined;
						current_meta_type.typeRange = rangess;
						current_meta_type.arrayRange = rangess;
						current_meta_type.isArraySizeExist = false;
						current_meta_type.isArrayExist = false;
						current_meta_type.typeName = namess;
					}					
			}																																								
			| simple LB FROM LB ALPHABET RB RB OPTIONAL { 
					printf("\n\tpostfix 12");
					isNameIsAlphabet = true;
					current_name = *($5);
					if (!isPostfixMeta) 					
					{
						VS_AsnCompilerRange rangess;
						world.AddAlphabetLinePostfix( *($5), true , current_type ,false, rangess , true );///5?					
					}
					else 
					{	VS_AsnCompilerName namess;
						isTypeFilled = true;
						VS_AsnCompilerRange rangess;
						//current_type = *($1);
						current_meta_type.type = current_type;
						current_meta_type.isTypeSizeExist = false;
						current_meta_type.arrayType = e_undefined;
						current_meta_type.typeRange = rangess;
						current_meta_type.arrayRange = rangess;
						current_meta_type.isArraySizeExist = false;
						current_meta_type.isArrayExist = false;
						current_meta_type.typeName = namess;
					}					
			}																																								
			| simple LB SIZE LB RANGE RB RB LB FROM LB ALPHABET RB RB { 
					printf("\n\tpostfix 13");
					isNameIsAlphabet = true;			
					VS_AsnCompilerName namess;
					current_name = *($11);	
					if (!isPostfixMeta) 					
					{
						world.AddAlphabetLinePostfix( *($11), true , current_type ,true, *($5) , false );///5?					
					}
					else 
					{	isTypeFilled = true;
						VS_AsnCompilerRange rangess;
						//current_type = *($1);
						current_meta_type.type = current_type;
						current_meta_type.isTypeSizeExist = true;
						current_meta_type.arrayType = e_undefined;
						current_meta_type.typeRange = *($5);
						current_meta_type.arrayRange = rangess;
						current_meta_type.isArraySizeExist = false;
						current_meta_type.isArrayExist = false;
						current_meta_type.typeName = namess;
					}																				
			}			
			| simple LB SIZE LB RANGE RB RB LB FROM LB ALPHABET RB RB OPTIONAL { 
					printf("\n\tpostfix 14");
					isNameIsAlphabet = true;			
					VS_AsnCompilerName namess;
					current_name = *($11);	
					if (!isPostfixMeta) 					
					{
						world.AddAlphabetLinePostfix( *($11), true , current_type ,false, *($5) , true );///5?					
					}
					else 
					{	isTypeFilled = true;
						VS_AsnCompilerRange rangess;
						//current_type = *($1);
						current_meta_type.type = current_type;
						current_meta_type.isTypeSizeExist = true;
						current_meta_type.arrayType = e_undefined;
						current_meta_type.typeRange = *($5);
						current_meta_type.arrayRange = rangess;
						current_meta_type.isArraySizeExist = false;
						current_meta_type.isArrayExist = false;
						current_meta_type.typeName = namess;
					}																				
			}
			| crypto_tags LBRACE TYPENAME RBRACE	{
					printf("\n\tpostfix 47");
					isNameIsAlphabet = false;
					if (!isPostfixMeta) 
					{
						//world.AddBadPostfix(  );
						VS_AsnCompilerRange rangess;
						VS_AsnSimpleTypes   typess = current_type;
						isNameIsAlphabet = false;
						world.AddLinePostfixCrypto(*($3), false ,typess ,false,rangess,false );
					}
					else 
					{
						printf("\n\t SYNTAXIs ERROR i n postfix7!!!!");
						exit(0);
					}	
													}
			| crypto_tags LBRACE TYPENAME RBRACE OPTIONAL	{
					printf("\n\tpostfix 48");
					isNameIsAlphabet = false;
					if (!isPostfixMeta) 
					{
						//world.AddBadPostfix(  );
						VS_AsnCompilerRange rangess;
						VS_AsnSimpleTypes   typess = current_type;
						isNameIsAlphabet = false;
						world.AddLinePostfixCrypto(*($3), false ,typess ,false,rangess,true );
					}
					else 
					{
						printf("\n\t SYNTAXIs ERROR i n postfix7!!!!");
						exit(0);
					}	
			
													}
			| crypto_tags LBRACE TYPENAME DASH_AND_DASH RBRACE	{ 
					printf("\n\tpostfix 49");
					isNameIsAlphabet = false;
					if (!isPostfixMeta) 
					{
						//world.AddBadPostfix(  );
						VS_AsnCompilerRange rangess;
						VS_AsnSimpleTypes   typess = current_type;
						printf("\n\t types: %d",typess);
						isNameIsAlphabet = false;
						world.AddLinePostfixCrypto(*($3), false ,typess ,false,rangess,false );
					}
					else 
					{
						printf("\n\t SYNTAXIs ERROR i n postfix7!!!!");
						exit(0);
					}	
			
													}																																															
			| crypto_tags LBRACE TYPENAME DASH_AND_DASH RBRACE OPTIONAL	{
					printf("\n\tpostfix 50");
					isNameIsAlphabet = false;
					if (!isPostfixMeta) 
					{
						//world.AddBadPostfix(  );
						VS_AsnCompilerRange rangess;
						VS_AsnSimpleTypes   typess = current_type;
						printf("\n\t types: %d",typess);
						isNameIsAlphabet = false;
						world.AddLinePostfixCrypto(*($3), false ,typess ,false,rangess,true );
					}
					else 
					{
						printf("\n\t SYNTAXIs ERROR i n postfix7!!!!");
						exit(0);
					}	
			}
			| TYPE_ID LB TYPENAME RB {
					printf("\n\tpostfix 51");
					isNameIsAlphabet = false;
					if (!isPostfixMeta) 
					{
						//world.AddBadPostfix(  );
						VS_AsnCompilerRange rangess;
						VS_AsnSimpleTypes   typess = e_typeid;
						printf("\n\t types: %d",typess);
						isNameIsAlphabet = false;
						world.AddLinePostfixCrypto(*($3), false ,typess ,false,rangess,false );
					}
					else 
					{
						isTypeFilled = true;
						VS_AsnCompilerRange rangess;
						//current_type = *($1);
						current_meta_type.type = e_typeid;
						current_meta_type.isTypeSizeExist = false;
						current_meta_type.arrayType = e_undefined;
						current_meta_type.typeRange = rangess;
						current_meta_type.arrayRange = rangess;
						current_meta_type.isArraySizeExist = false;
						current_meta_type.isArrayExist = false;
						current_meta_type.typeName = *($3);
						current_type = e_typeid;
					}				
			}
            ; 
crypto_tags :    HASHED			{current_type = e_hashed;		printf("\n\t\tHASHED");}
			|    ENCRYPTED		{current_type = e_encrypted;		printf("\n\t\tENCRYPTED");}
			|    SIGNED         {current_type = e_signed;		printf("\n\t\tSIGNED");}
			;



prefix      : VARNAME									{
					VS_AsnSimpleTypes typess = e_undefined;
					VS_AsnCompilerRange rangess;
					printf("\n\t prefix0");
					printf("\n\t prefix 0 %s",(*($1)).data);
					world.AddLinePrefix(*($1),false,false,typess,rangess );	
					isPostfixMeta = false;
					printf("\n\t prefix 1"); 
					//current_name = *($1);
														}
            | VARNAME array OF							{
					VS_AsnCompilerRange rangess ;
					printf("\n\tprefix 2");
					isPostfixMeta = false;
					world.AddLinePrefix(*($1),false,true,current_type,rangess );	
														}
            | VARNAME array SIZE LB RANGE RB OF	{
					printf("\n\tprefix 3");					
					isPostfixMeta = false;
					world.AddLinePrefix(*($1),false,true,current_type, *($5) );	
														}
            | VARNAME array LB SIZE LB RANGE RB RB OF	{
					printf("\n\tprefix 4");
					isPostfixMeta = false;
					world.AddLinePrefix(*($1),false,true,current_type, *($6) );	///$5=range???
														}
            ;
embdtypepref: VARNAME container RET LBRACE				{
					printf("\n\t embeded type prefix");	
					VS_AsnCompilerName namess = *($1);
					world.AddEmbededType( namess , current_type, false ); 
														}            
			| VARNAME container DASH_AND_DASH RET LBRACE				{
					printf("\n\t embeded type prefix");	
					VS_AsnCompilerName namess = *($1);
					world.AddEmbededType( namess , current_type, false ); 
														}            
			| embdtypepref DASH_AND_DASH	{ printf("\n\t embdtypepref DASH_AND_DASH"); }
			/*| VARNAME array SIZE LB RANGE RB OF container RET LBRACE	{
				printf("********++++++++********\n");
				if(current_type == e_sequence)
					printf("SEQUENCE\n");
				else if(current_type == e_set)
					printf("SET\n");
				else
					printf("current_type = %d\n",current_type);
				
										}*/
			;                                                                                          
array       : SEQUENCE	{current_type = e_sequence;		printf("\n\tSEQUENCE");	}
            | SET		{current_type = e_set;			printf("\n\tSET");    }
            ;
container   : SEQUENCE  {current_type = e_sequence;			printf("\n\t\tarray0");}      
			| SET		{current_type = e_set;			printf("\n\t\tarray1"); }      
            | CHOICE	{current_type = e_choice;			printf("\n\t\tchoice"); }
            ; 
simple      : INTEGER	{current_type = e_integer;		printf("\n\t\tint");}
            | BOOL		{current_type = e_bool;			printf("\n\t\tbool");}
            | NULLER	{current_type = e_null;			printf("\n\t\tnull");}
            | ENUM		{current_type = e_enumeration;	printf("\n\t\tenum");	}
            | OBJID		{current_type = e_objectid;		printf("\n\t\tobj");	}
            | BITSTRING	{current_type = e_bitstring;	printf("\n\t\tbit");	}
            | OCTSTRING	{current_type = e_octstring;	printf("\n\t\toct");	}
            | NUMSTRING	{current_type = e_numstring;	printf("\n\t\tnum");	}
            | IA5STRING	{current_type = e_ia5string;	printf("\n\t\tia5");	}
            | PRISTRING	{current_type = e_pristring;	printf("\n\t\tpri");	}
            | GENSTRING	{current_type = e_genstring;	printf("\n\t\tgenl");	}
            | BMPSTRING	{current_type = e_bmpstring;	printf("\n\t\tbmp");	}
            ;


%%

int yyerror(const char*  s)
{
	//printf("\n\t Syntaxis Error");
	//printf("%s",yytext);
    //return printf(" %s",s);
	return fprintf(stderr, "syntax error: %s\n", s);
}

