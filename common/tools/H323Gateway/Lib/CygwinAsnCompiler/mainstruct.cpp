#include "mainstruct.h"


bool operator < (const VS_AsnCompilerName &Left, const VS_AsnCompilerName &Right)
{
	int i = memcmp(Left.data,Right.data,sizeof(Left.data));
	if(i<0)
		return true;
	else
		return false;
}
int dddd = 0 ;
const char pream[]="\n/////////////////////////////////////////////////////////////////////////////////////////\nstruct ";
char prefix[MAX_NAME_SIZE]="VS_H323";
const char string_types[][MAX_NAME_SIZE]=
{	"###",
	"ERROR0-type.id",
	"ERROR1-hashed",
	"ERROR2-signed",
	"ERROR3-encrypted",
	"VS_AsnNull",
	"VS_AsnBoolean",
	"VS_AsnInteger",
	"VS_AsnEnumeration",
	"VS_AsnObjectId",
	"VS_AsnBitString",
	"VS_AsnOctetString",
	"VS_AsnNumericString",
	"VS_AsnIA5String",
	"VS_AsnPrintableString",
	"VS_AsnGeneralString",
	"VS_AsnBmpString",
	"VS_AsnChoice",
	"VS_AsnSequence",
	"VS_AsnSet",
	"Array_of_type",
	"Constrained_array_of_type"
};
// sequence
		const char seq_r[]="ref";
		const char seq_b[]="basic_root";
		const char seq_e[]="e_ref";
		const char seq_x[]="extension_root";
//chice
const unsigned long DEFAULT_SIZE = 1024;

////////////////////////////////////////////////////////////////////////////////////////
void VS_AsnMetaTypeCompilerContainer::MakeShortWalk( VS_AsnCompilerContainer &container,
											         VS_AsnCompilerBuffer &buffer,
													 VS_AsnCompilerBuffer &buffer_cpp)
{
	//unsigned d=0;
//	printf("\nSHORT1");
	printf("\n\t SHORT CURRENT :%s",this->typeName.data);
	for(walk_line=0;walk_line<current_line;walk_line++)
	{
		if (lines[ walk_line ].isEmbeded)
		{
		//	printf("\n\tSHORT In Embeded");
		//	printf("\n\tSHORT I`m in!: %s",this->typeName.data);
		//	printf("\n\tSHORT Key    : %d",this->myKey);
		//	printf("\n\tSHORT FKey   : %d",this->fathersKey );
		//	printf("\n\tSHORT index  : %d max: %d",walk_line,current_line);
		//	printf("\n\tSHORT EmbededKey   :%D",lines[ walk_line ].myKey);
			//lines[ walk_line ].Show();
			VS_AsnCompilerKey key = lines[ walk_line ].myKey;
			container.class_tree[ key ].MakeShortWalk( container , buffer , buffer_cpp );
		}
		////scanf("%d",&d);
	}
//	printf("\n\t SECOND SHORT CURRENT :%s",this->typeName.data);
//	printf("\nSHORT2");
	int result=0;
	result =  MakeHeadersStructs( buffer , buffer_cpp );
//	printf("\nSHORT3");
	if (result!=1)
	{
		printf("\n\t Error in MakeSHORTWalk - MakeHeadersStructs res = %d",result);
	}
//	printf("\n\t EXIT from MakeSHORTWalk");

}
/////////////////////////////////////////////////////////////////////////////////////////

void VS_AsnMetaTypeCompilerContainer::MakeWalk( VS_AsnCompilerContainer &container,
											    VS_AsnCompilerBuffer &buffer,
												VS_AsnCompilerBuffer &buffer_cpp)
{
	//unsigned d=0;
//	printf("\n1");
	printf("\n\t CURRENT :%s",this->typeName.data);
	for(walk_line=0;walk_line<current_line;walk_line++)
	{
		if (lines[ walk_line ].isEmbeded)
		{

//			printf("\n\t In Embeded");
//			printf("\n\t I`m in!: %s",this->typeName.data);
//			printf("\n\t Key    : %d",this->myKey);
//			printf("\n\t FKey   : %d",this->fathersKey );
//			printf("\n\t index  : %d max: %d",walk_line,current_line);
//			printf("\n\t EmbededKey   :%D",lines[ walk_line ].myKey);
			//lines[ walk_line ].Show();
			VS_AsnCompilerKey key = lines[ walk_line ].myKey;
			container.class_tree[ key ].MakeWalk( container , buffer , buffer_cpp );
		}
		////scanf("%d",&d);
	}
//	printf("\n\t SECOND CURRENT :%s",this->typeName.data);
//	printf("\n2");
	int result=0;
	result =  MakeClass( buffer , buffer_cpp );
//	printf("\n3");
	if (result!=1)
	{
		printf("\n\t Error in MakeWalk - MakeClass res = %d",result);
	}
//	printf("\n\t EXIT from MakeWalk");

}
/////////////////////////////////////////////////////////////////////////////////////////
VS_AsnCompilerName VS_AsnCompilerVarNameCorrector(VS_AsnCompilerName sname)
{
	unsigned i=0;
	VS_AsnCompilerName name = sname;
	if ((name.data[0] >= 65) && (name.data[0] <= 90)) name.data[0]+=32;
	for(i=0;i<strlen(name.data);i++)
		if ('-'==name.data[i]) name.data[i]='_';
	return name;
}
/////////////////////////////////////////////////////////////////////////////////////////
VS_AsnCompilerName VS_AsnCompilerNameCorrector(VS_AsnCompilerName sname)
{
	unsigned i=0;
	VS_AsnCompilerName name = sname;
	if ((name.data[0] >= 97) && (name.data[0] <= 122)) name.data[0]-=32;
	for(i=0;i<strlen(name.data);i++)
		if ('-'==name.data[i]) name.data[i]='_';
	return name;
}
void VS_AsnCompilerNameTranslator( VS_AsnCompilerName name ,VS_AsnCompilerName &vs_name )
{
	VS_AsnCompilerName tmp;
	tmp = VS_AsnCompilerNameCorrector( name );
	snprintf(vs_name.data,MAX_NAME_SIZE,"%s%s",prefix,tmp.data);
}
/////////////////////////////////////////////////////////////////////////////////////////
void VS_AsnCompilerNameTranslator(VS_AsnCompilerName &name)
{	VS_AsnCompilerName tmp;
	VS_AsnCompilerNameTranslator( name , tmp );
	name = tmp;
}
/////////////////////////////////////////////////////////////////////////////////////////
void VS_AsnCompilerEmbededNameTranslator(VS_AsnCompilerName &name , VS_AsnCompilerName fathers_name)
{	VS_AsnCompilerName tmp;
	name = VS_AsnCompilerNameCorrector( name );
	snprintf(tmp.data,MAX_NAME_SIZE,"%s_%s",fathers_name.data,name.data);
	name = tmp;
}
////////////////////////////////////////////////////////////////////////////////////////
int VS_AsnMetaTypeCompilerContainer::MakeHeadersStructs(
							VS_AsnCompilerBuffer &buffer ,
							VS_AsnCompilerBuffer &byffer_cpp)
{
	ZeroTmp();
	//int res = 0;
	switch( type )
	{
	case e_hashed:
	case e_signed:
	case e_encrypted:
	case e_typeid:
	case e_null:
	case e_bool:
	case e_integer:
	case e_enumeration:
	case e_objectid:
	case e_bitstring:
	case e_octstring:
	case e_bmpstring:
	case e_sequence_of:
	case e_set_of:
		{

		} break;
	case e_pristring:
	case e_genstring:
	case e_ia5string:
	case e_numstring:
		{

		} break;
	case e_choice:
	case e_sequence:
	case e_set:
		{
			ZeroTmp();
			snprintf(tmp,MAX_NAME_SIZE,"\t struct %s ;\n" ,typeName.data );
			buffer.AddString( tmp );
		}break;
	default:
		{
			printf("\n\t UNKNOWN TYPE! error in MHEADERPreambule");
			return 0;
		}
	}
	return 1;
}
////////////////////////////////////////////////////////////////////////////////////////
inline int VS_AsnMetaTypeCompilerContainer::MakeClass(VS_AsnCompilerBuffer &buffer
													  , VS_AsnCompilerBuffer &buffer_cpp)
{
	ZeroTmp();
	int res = 0;
	switch( type )
	{
	case e_hashed:
	case e_signed:
	case e_encrypted:
	case e_typeid:
	case e_null:
	case e_bool:
	case e_integer:
	case e_enumeration:
	case e_objectid:
	case e_bitstring:
	case e_octstring:
	case e_bmpstring:
	case e_sequence_of:
	case e_set_of:
	case e_undefined:
		{
			///typedef
			res = MHEADERPreambule_simple( buffer );
			///////////////
			if (res==1)
			{
				res = MCPPPreambule_simple( buffer_cpp );
				info("MCPPPreambule_simple \t passed.");
			}
			else {	printf("\n\t ERROR 1 in MakeClass. res = %d",res); exit(0); }
		}break;
	case e_pristring:
	case e_genstring:
	case e_numstring:
	case e_ia5string:
		{
			if (this->isAlphabetExist)
			{   ///This is not joke! This may be will be userfull in future. ALEX.
				res = MHEADERPreambule_strings( buffer );
				///////////////
				if (res==1)
				{
					res = MCPPPreambule_strings( buffer_cpp );
					info("MHEADERPreambule_strings \t passed.");
				}
				else {	printf("\n\t ERROR 1 in MakeClass. res = %d",res); exit(0); }
				///////////////
				if (res==1)
				{
					res = MHEADERConstructorDeclaration_strings( buffer );
					info("MCPPPreambule_strings \t passed.");
				}
				else {	printf("\n\t ERROR 2 in MakeClass. res = %d",res); exit(0); }
				///////////////
				if (res==1)
				{
					res = MCPPConstructorRealisation_strings( buffer_cpp );
					info("MHEADERConstructorDeclaration_strings \t passed.");
				}
				else {	printf("\n\t ERROR 3 in MakeClass. res = %d",res); exit(0); }
				///////////////
				if (res==1)
				{
					res = MCPPConstructorsBody_strings( buffer_cpp );
					info("MCPPConstructorRealisation_strings \t passed.");
				}
				else {	printf("\n\t ERROR 4 in MakeClass. res = %d",res); exit(0); }
				///////////////
				if (res==1)
				{
					res = MHEADERClassBody_strings( buffer );
					info("MCPPConstructorsBody_strings \t passed.");
				}
				else {	printf("\n\t ERROR 5 in MakeClass. res = %d",res); exit(0); }
				///////////////
				if (res==1)
				{
					res = MCPPClassBody_strings( buffer_cpp );
					info("MHEADERClassBody_strings \t passed.");
				}
				else {	printf("\n\t ERROR 6 in MakeClass. res = %d",res); exit(0); }
				///////////////
				if (res==1)
				{
					res = MHEADERClassMethods_strings( buffer );
					info("MCPPClassBody_strings \t passed.");
				}
				else {	printf("\n\t ERROR 7 in MakeClass. res = %d",res); exit(0); }
				///////////////
				if (res==1)
				{
					res = MCPPClassMethods_strings( buffer_cpp );
					info("MHEADERClassMethods_strings \t passed.");
				}
				else {	printf("\n\t ERROR 8 in MakeClass. res = %d",res); exit(0); }
				info("MCPPClassMethods_strings \t passed.");
			}
			else
			{
			///typedef
				res = MHEADERPreambule_simple( buffer );
				///////////////
				if (res==1)
				{
					res = MCPPPreambule_simple( buffer_cpp );
					info("MCPPPreambule_simple \t passed.");
				}
				else
				{
					printf("\n\t ERROR 1 in MakeClass. res = %d",res);
					exit(0);
				}

			}
		}break;
	case e_choice:
		{
			res = MHEADERPreambule_choice( buffer );
			///////////////
			if (res==1)
			{
				res = MCPPPreambule_choice( buffer_cpp );
				info("MHEADERPreambule_choice \t passed.");
			}
			else {	printf("\n\t ERROR 11 in MakeClass. res = %d",res); exit(0); }
			///////////////
			if (res==1)
			{
				res = MHEADERConstructorDeclaration_choice( buffer );
				info("MCPPPreambule_choice \t passed.");
			}
			else {	printf("\n\t ERROR 12 in MakeClass. res = %d",res); exit(0); }
			///////////////
			if (res==1)
			{
				res = MCPPConstructorRealisation_choice( buffer_cpp );
				info("MHEADERConstructorDeclaration_choice \t passed.");
			}
			else {	printf("\n\t ERROR 13 in MakeClass. res = %d",res); exit(0); }
			///////////////
			if (res==1)
			{
				res = MCPPConstructorsBody_choice( buffer_cpp );
				info("MCPPConstructorRealisation_choice \t passed.");
			}
			else {	printf("\n\t ERROR 14 in MakeClass. res = %d",res); exit(0); }
			///////////////
			if (res==1)
			{
				res = MHEADERClassBody_choice( buffer );
				info("MCPPConstructorsBody_choice \t passed.");
			}
			else {	printf("\n\t ERROR 15 in MakeClass. res = %d",res); exit(0); }
			///////////////
			if (res==1)
			{
				res = MCPPClassBody_choice( buffer_cpp );
				info("MHEADERClassBody_choice \t passed.");
			}
			else {	printf("\n\t ERROR 16 in MakeClass. res = %d",res); exit(0); }
			///////////////
			if (res==1)
			{
				res = MHEADERClassMethods_choice( buffer );
				info("MCPPClassBody_choice \t passed.");
			}
			else {	printf("\n\t ERROR 17 in MakeClass. res = %d",res); exit(0); }
			///////////////
			if (res==1)
			{
				res = MCPPClassMethods_choice( buffer_cpp );
				info("MHEADERClassMethods_choice \t passed.");
			}
			else {	printf("\n\t ERROR 18 in MakeClass. res = %d",res); exit(0); }
			info("MCPPClassMethods_choice \t passed.");

		}break;
	case e_sequence:
	case e_set:
		{
			res = MHEADERPreambule_sequence( buffer );
			///////////////
			if (res==1)
			{
				res = MCPPPreambule_sequence( buffer_cpp );
				info("MHEADERPreambule_sequence \t passed.");
			}
			else {	printf("\n\t ERROR 11 in MakeClass. res = %d",res); exit(0); }
			///////////////
			if (res==1)
			{
				res = MHEADERConstructorDeclaration_sequence( buffer );
				info("MCPPPreambule_sequence \t passed.");
			}
			else {	printf("\n\t ERROR 12 in MakeClass. res = %d",res); exit(0); }
			///////////////
			if (res==1)
			{
				res = MCPPConstructorRealisation_sequence( buffer_cpp );
				info("MHEADERConstructorDeclaration_sequence \t passed.");
			}
			else {	printf("\n\t ERROR 13 in MakeClass. res = %d",res); exit(0); }
			///////////////
			if (res==1)
			{
				res = MCPPConstructorsBody_sequence( buffer_cpp );
				info("MCPPConstructorRealisation_sequence \t passed.");
			}
			else {	printf("\n\t ERROR 14 in MakeClass. res = %d",res); exit(0); }
			///////////////
			if (res==1)
			{
				res = MHEADERClassBody_sequence( buffer );
				info("MCPPConstructorsBody_sequence \t passed.");
			}
			else {	printf("\n\t ERROR 15 in MakeClass. res = %d",res); exit(0); }
			///////////////
			if (res==1)
			{
				res = MCPPClassBody_sequence( buffer_cpp );
				info("MHEADERClassBody_sequence \t passed.");
			}
			else {	printf("\n\t ERROR 16 in MakeClass. res = %d",res); exit(0); }
			///////////////
			if (res==1)
			{
				res = MHEADERClassMethods_sequence( buffer );
				info("MCPPClassBody_sequence \t passed.");
			}
			else {	printf("\n\t ERROR 17 in MakeClass. res = %d",res); exit(0); }
			///////////////
			if (res==1)
			{
				res = MCPPClassMethods_sequence( buffer_cpp );
				info("MHEADERClassMethods_sequence \t passed.");
			}
			else {	printf("\n\t ERROR 18 in MakeClass. res = %d",res); exit(0); }
			info("MCPPClassMethods_sequence \t passed.");

		}break;
	default:
		{
			printf("\n\t UNKNOWN TYPE! error in MHEADERPreambule : %d", type );
			return 0;
		}
	}
	return 1;
}
///////////////////////////////////////////////////////////////////////////////////////
inline int VS_AsnMetaTypeCompilerContainer::MHEADERPreambule_sequence(
								VS_AsnCompilerBuffer &buffer)
{
	ZeroTmp();
	snprintf(tmp,MAX_NAME_SIZE,"\n//////////////////////CLASS %s /////////////////////////\n " ,typeName.data );
	buffer.AddString( tmp );
	snprintf(tmp,MAX_NAME_SIZE,"\nstruct %s : public VS_AsnSequence\n{\n" ,typeName.data );
	buffer.AddString( tmp );
	return 1;
}
////////////////////////
inline int VS_AsnMetaTypeCompilerContainer::MCPPPreambule_sequence(
								VS_AsnCompilerBuffer &buffer_cpp)
{
	ZeroTmp();
	snprintf(tmp,MAX_NAME_SIZE,
		"\n//////////////////////CLASS %s /////////////////////////\n " ,typeName.data );
	buffer_cpp.AddString( tmp );
	return 1;
}
///////////////////////////////////////////////////////////////////////////////////////
inline int VS_AsnMetaTypeCompilerContainer::MHEADERPreambule_choice(
								VS_AsnCompilerBuffer &buffer )
{
	ZeroTmp();
	snprintf(tmp,MAX_NAME_SIZE,"\n//////////////////////CLASS %s /////////////////////////\n " ,typeName.data );
	buffer.AddString( tmp );
	snprintf(tmp,MAX_NAME_SIZE,"\nstruct %s : public VS_AsnChoice\n{\n" ,typeName.data );
	buffer.AddString( tmp );
	return 1;

}
////////////////////////
inline int VS_AsnMetaTypeCompilerContainer::MCPPPreambule_choice(
								VS_AsnCompilerBuffer &buffer_cpp )
{
	ZeroTmp();
	snprintf(tmp,MAX_NAME_SIZE,
		"\n//////////////////////CLASS %s /////////////////////////\n " ,typeName.data );
	buffer_cpp.AddString( tmp );
	return 1;

}
///////////////////////////////////////////////////////////////////////////////////////
inline int VS_AsnMetaTypeCompilerContainer::MHEADERPreambule_strings(
								VS_AsnCompilerBuffer &buffer )
{
	ZeroTmp();
	snprintf(tmp,MAX_NAME_SIZE,"\n//////////////////////CLASS %s /////////////////////////\n " ,typeName.data );
	buffer.AddString( tmp );

	snprintf(tmp,MAX_NAME_SIZE,"\nstruct %s : public VS_AsnRestrictedString \n{\n" ,typeName.data );
	buffer.AddString( tmp );
	return 1;
}
////////////////////////
inline int VS_AsnMetaTypeCompilerContainer::MCPPPreambule_strings(
								VS_AsnCompilerBuffer &buffer_cpp )
{
	ZeroTmp();
	snprintf(tmp,MAX_NAME_SIZE,
		"\n//////////////////////CLASS %s /////////////////////////\n " ,typeName.data );
	buffer_cpp.AddString( tmp );
	return 1;
}
///////////////////////////////////////////////////////////////////////////////////////
inline int VS_AsnMetaTypeCompilerContainer::MHEADERPreambule_simple(
								VS_AsnCompilerBuffer &buffer )
{	ZeroTmp();

	VS_AsnCompilerName tmpName;

	snprintf(tmp,MAX_NAME_SIZE,"\n//////////////////////CLASS %s /////////////////////////\n " ,typeName.data );
	buffer.AddString( tmp );
	///////////strings
	if (this->isAlphabetExist)
	{
		lo("Im in :");
		lo(typeName.data);
		res = MHEADERAlphabet( buffer , typeName,typeName , 0);
		if (res!=1) DError(" MHEADERPreambule_simple - MCPPAlphabet ",res);
		return res;
	}
	/////////////////////
	snprintf(tmp,MAX_NAME_SIZE,"\ntypedef ");
	buffer.AddString( tmp );

	tmpName = typeName;
	if (((type==e_undefined)&&((isEmbeded==true)||(isArrayExist)||(isTypeSizeExist)))||(type!=e_undefined))
		res = MTypeName( buffer , tmpName , 0);
	else
	{
		res = 1;
		snprintf(tmp,MAX_NAME_SIZE," %s ",simple_meta_name.data);//tmpName.data );
		buffer.AddString(tmp);
	}
	if (1!=res) DError("MHEADERPreambule_simple - MTypeName",res);
	if (type==e_undefined)
		snprintf(tmp,MAX_NAME_SIZE," %s; ",tmpName.data);//simple_meta_name.data);
	else
		snprintf(tmp,MAX_NAME_SIZE," %s; ",typeName.data);
	buffer.AddString( tmp );
	return 1;
}
inline int VS_AsnMetaTypeCompilerContainer::MCPPPreambule_simple(
								VS_AsnCompilerBuffer &buffer_cpp )
{	ZeroTmp();
	///////////strings all
	if (this->isAlphabetExist)
	{	lo("Im in :");
		lo(typeName.data);
		res = MCPPAlphabet( buffer_cpp , typeName,typeName , 0);
		if (res!=1) DError(" MCPPPreambule_simple - MCPPAlphabet ",res);
		return res;
	}
	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////
inline int VS_AsnMetaTypeCompilerContainer::MHEADERConstructorDeclaration_strings(
								VS_AsnCompilerBuffer &buffer )
{	ZeroTmp();
	snprintf(tmp,MAX_NAME_SIZE,"\t %s( void );\n ",typeName.data);
	buffer.AddString( tmp );
	return 1;
}
////////////////////////
inline int VS_AsnMetaTypeCompilerContainer::MCPPConstructorRealisation_strings(
								VS_AsnCompilerBuffer &buffer_cpp )
{	ZeroTmp();
	snprintf(tmp,MAX_NAME_SIZE,"\t %s::%s( void ):\n",typeName.data,typeName.data);
	buffer_cpp.AddString( tmp );
	//
	snprintf(tmp,
	MAX_NAME_SIZE,"\t VS_AsnRestrictedString( %s_alphabet,\n\t\t%s_alphabet_size,\n\t\t%s_inverse_table,",
		typeName.data,typeName.data,typeName.data);
	buffer_cpp.AddString( tmp );
	//
	if (isTypeSizeExist)
	{
		snprintf(tmp,
			MAX_NAME_SIZE,"%d,%lu,VS_Asn::FixedConstraint,%d)\n\t{}\n",
			typeRange.low,typeRange.upp,typeRange.isExtendable);
	}
	else
	{
		snprintf(tmp,
			MAX_NAME_SIZE,"0,INT_MAX,VS_Asn::Unconstrained,%d)\n\t{}\n",
			typeRange.isExtendable);

	}
	buffer_cpp.AddString( tmp );
	return 1;
}
///////////////////////////////////////////////////////////////////////////////////////
inline int VS_AsnMetaTypeCompilerContainer::MHEADERConstructorDeclaration_choice(
								VS_AsnCompilerBuffer &buffer )
{	ZeroTmp();
	snprintf(tmp,MAX_NAME_SIZE,
	"\t %s( void );\n",
	typeName.data );
	buffer.AddString( tmp );
	return 1;
}
////////////////////////
inline int VS_AsnMetaTypeCompilerContainer::MCPPConstructorRealisation_choice(
								VS_AsnCompilerBuffer &buffer_cpp )
{	ZeroTmp();
	snprintf(tmp,MAX_NAME_SIZE,"\t %s::%s( void )\n\t:VS_AsnChoice(%lu , %lu , %d)\n\t{\n",
		typeName.data,
		typeName.data,
		num_of_simple,
		num_of_simple+num_of_extension,
		isExtendable);
	buffer_cpp.AddString( tmp );
	return 1;
}
///////////////////////////////////////////////////////////////////////////////////////
inline int VS_AsnMetaTypeCompilerContainer::MHEADERConstructorDeclaration_sequence(
								VS_AsnCompilerBuffer &buffer )
{	ZeroTmp();
	snprintf(tmp,MAX_NAME_SIZE,
		"\t %s( void );\n",
	     typeName.data );
	buffer.AddString( tmp );
	return 1;
}
////////////////////////
inline int VS_AsnMetaTypeCompilerContainer::MCPPConstructorRealisation_sequence(
								VS_AsnCompilerBuffer &buffer_cpp )
{	ZeroTmp();
	snprintf(tmp,MAX_NAME_SIZE,
		"\t %s :: %s( void )\n\t:VS_AsnSequence(%lu , %s , %s, %s , %s , %d )\n\t{\n",
				typeName.data,
				typeName.data,
				num_of_options,
				seq_r,
				seq_b,
				seq_e,
				seq_x,
				isExtendable);
	buffer_cpp.AddString( tmp );
	return 1;
}
///////////////////////////////////////////////////////////////////////////////////////
inline int VS_AsnMetaTypeCompilerContainer::MCPPConstructorsBody_sequence(
								VS_AsnCompilerBuffer &buffer_cpp )
{	ZeroTmp();
	if ((type!=e_sequence) && (type!=e_choice) && (type!=e_set)) 		return 0;

	if ((e_sequence==type) || (e_set==type))
	{
		unsigned int i=0;
		for(i=0;i<num_of_simple;i++)
		{
			snprintf(tmp,MAX_NAME_SIZE,"\t\t%s[%d].Set(&%s,%d);\n"
				,seq_r
				,i
				,lines[ i ].varName.data
				,lines[ i ].isOptional);
			buffer_cpp.AddString( tmp );
		}
		if (isExtendable)
		{
			for(i=num_of_simple;i<current_line;i++)
			{
				snprintf(tmp,MAX_NAME_SIZE,"\t\t%s[%lu].Set(&%s,%d);\n"
					,seq_e
					,i - num_of_simple
					,lines[ i ].varName.data
					,lines[ i ].isOptional);
				buffer_cpp.AddString( tmp );
	}	}	}

	snprintf(tmp,MAX_NAME_SIZE,"\t}\n");
	buffer_cpp.AddString( tmp );
	return 1;
}
///////////////////////////////////////////////////////////////////////////////////////
inline int VS_AsnMetaTypeCompilerContainer::MCPPConstructorsBody_choice(
								VS_AsnCompilerBuffer &buffer_cpp )
{	ZeroTmp();
	snprintf(tmp,MAX_NAME_SIZE,"\t}\n");
	buffer_cpp.AddString( tmp );
	return 1;
}
///////////////////////////////////////////////////////////////////////////////////////
inline int VS_AsnMetaTypeCompilerContainer::MCPPConstructorsBody_strings(
								VS_AsnCompilerBuffer &buffer_cpp )
{	ZeroTmp();///?????
	return 1;
}
///////////////////////////////////////////////////////////////////////////////////////
inline int VS_AsnMetaTypeCompilerContainer::MHEADERClassBody_sequence(
								VS_AsnCompilerBuffer &buffer )
{	ZeroTmp();
	if ((e_sequence==type) || (e_set==type))
	{
		snprintf(tmp,MAX_NAME_SIZE,"\n\tstatic const unsigned %s = %lu;",seq_b,num_of_simple);
		buffer.AddString( tmp );
		if(num_of_simple)
			snprintf(tmp,MAX_NAME_SIZE,"\n\tVS_Reference_of_Asn %s[%s];",seq_r,seq_b);
		else
			snprintf(tmp,MAX_NAME_SIZE,"\n\tVS_Reference_of_Asn* %s;",seq_r);
		buffer.AddString( tmp );
		snprintf(tmp,MAX_NAME_SIZE,"\n\tstatic const unsigned %s = %lu;",seq_x,num_of_extension);
		buffer.AddString( tmp );
		if(num_of_extension)
			snprintf(tmp,MAX_NAME_SIZE,"\n\tVS_Reference_of_Asn %s[%s];",seq_e,seq_x);
		else
			snprintf(tmp,MAX_NAME_SIZE,"\n\tVS_Reference_of_Asn* %s;",seq_e);
		buffer.AddString( tmp );
		snprintf(tmp,MAX_NAME_SIZE,"\n\n");
		buffer.AddString( tmp );
		//char tmp1[MAX_NAME_SIZE]={0};

		////ALPhabet
		unsigned long i=0;
		for(i=0;i<current_line;i++)
		{
			res = lines[ i ].MHEADERAlphabet( buffer, lines[ i ].varName, typeName , 0 );
			if ((res!=1)&&(res!=-1))
			{
				DError(" MHEADERClassBody_sequence in MHEADERAlphabet",res);
			}
		}
		////ALPhabet end

		for(i=0;i<num_of_simple;i++)
		{
			//printf("\n\t lines : %d",i);
			res = lines[ i ].MakeLine( buffer );
			if (res!=1)
			{	DError(" MHEADERClassBody_sequence in MakeLine",res);	}
			//snprintf(tmp,MAX_NAME_SIZE,"\n\t%s",tmp1);
			//buffer.AddString( tmp );
		}
		//l("113");
		if (isExtendable)
		{
			snprintf(tmp,MAX_NAME_SIZE,"\n\t \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\");
			//printf("\n\t %d ",current_line - num_of_simple);
			for(i=num_of_simple;i<current_line;i++)
			{
				lo("113.3");
				res = lines[ i ].MakeLine( buffer );
				if (res!=1)
				{
					lines[ i ].Show();
					DError(" MHEADERClassBody_sequence in MakeLine",res);
				}
				lo("113.4");
				//snprintf(tmp,MAX_NAME_SIZE,"\n\t%s",tmp1);
				//lo("113.5");
				//buffer.AddString( tmp );
				//lo("113.6");
			}
		}
		//l("114");
	}
	else
	{
		DError(" MHEADERClassBody_sequence in else ",111);
	}
	return 1;
}
///////////
inline int VS_AsnMetaTypeCompilerContainer::MCPPClassBody_sequence(
								VS_AsnCompilerBuffer &buffer_cpp )
{	ZeroTmp();
	unsigned long i;
	for(i=0;i<current_line;i++)
	{
		res = lines[ i ].MCPPAlphabet( buffer_cpp, lines[ i ].varName, typeName , 0 );
		if ((res!=1)&&(res!=-1))
		{
			DError(" MHEADERClassBody_sequence in MHEADERAlphabet",res);
		}
	}
	return 1;
}
///////////////////////////////////////////////////////////////////////////////////////
inline int VS_AsnMetaTypeCompilerContainer::MHEADERClassBody_choice(
								VS_AsnCompilerBuffer &buffer )
{	ZeroTmp();
	////ALPhabet
	unsigned long i=0;
	for(i=0;i<current_line;i++)
	{
		res = lines[ i ].MHEADERAlphabet( buffer, lines[ i ].varName, typeName , 0 );
		if ((res!=1)&&(res!=-1))
		{
			DError(" MHEADERClassBody_CHOICE in MHEADERAlphabet",res);
		}
	}
	////ALPhabet end
	return 1;
}
///////////
inline int VS_AsnMetaTypeCompilerContainer::MCPPClassBody_choice(
								VS_AsnCompilerBuffer &buffer_cpp )
{	ZeroTmp();
	unsigned long i;
	for(i=0;i<current_line;i++)
	{
		res = lines[ i ].MCPPAlphabet( buffer_cpp, lines[ i ].varName, typeName , 0 );
		if ((res!=1)&&(res!=-1))
		{
			DError(" MHEADERClassBody_sequence in MHEADERAlphabet",res);
		}
	}
	return 1;

}
///////////////////////////////////////////////////////////////////////////////////////
inline int VS_AsnMetaTypeCompilerContainer::MHEADERClassBody_strings(
								VS_AsnCompilerBuffer &buffer )
{	ZeroTmp();
	res = MHEADERAlphabet( buffer , typeName , typeName ,0);
	if (res!=1) DError(" MHEADERClassBody_strings - MHEADERAlphabet",res);
	return 1;
}

inline int VS_AsnMetaTypeCompilerContainer::MCPPClassBody_strings(
								VS_AsnCompilerBuffer &buffer_cpp )
{	ZeroTmp();
	res = MCPPAlphabet( buffer_cpp , typeName , typeName,0 );
	if (res!=1) DError(" MCPPClassBody_strings - MCPPAlphabet",res);
	return 1;
}
///////////////////////////////////////////////////////////////////////////////////////
inline int VS_AsnMetaTypeCompilerContainer::MHEADERClassMethods_sequence(
								VS_AsnCompilerBuffer &buffer )
{	ZeroTmp();
	snprintf(tmp,MAX_NAME_SIZE,"\tvoid operator=(const %s& src);\n",typeName.data);
	buffer.AddString( tmp );

/*
	snprintf(tmp,MAX_NAME_SIZE,"\tbool operator==(const %s& src);\n"
		,typeName.data);
	buffer.AddString( tmp );
*/
	buffer.AddString("\n};");


	return 1;
}

inline int VS_AsnMetaTypeCompilerContainer::MCPPClassMethods_sequence(
								VS_AsnCompilerBuffer &buffer_cpp )
{	ZeroTmp();
	snprintf(tmp,MAX_NAME_SIZE,"\tvoid %s::operator=(const %s& src)\n\t{\n\t",typeName.data,typeName.data);
	buffer_cpp.AddString( tmp );
	unsigned long i;
	snprintf(tmp,MAX_NAME_SIZE,"\n\t\tO_CC(filled);");
	buffer_cpp.AddString( tmp );
	for(i=0;i<current_line;i++)
	{
		snprintf(tmp,MAX_NAME_SIZE,"\n\t\tO_C(%s);",lines[ i ].varName.data);
		buffer_cpp.AddString( tmp );
	}
	buffer_cpp.AddString( "\n\t}\n" );
	/////////
	snprintf(tmp,MAX_NAME_SIZE,"\n ///////////////////////////////////////////////////////////////////////////////////////// \n");
	buffer_cpp.AddString( tmp );
	/////////
	/*
	snprintf(tmp,MAX_NAME_SIZE,"\tbool %s::operator==(const %s& src)\n\t{\n\t\tbool res = src.filled;\n "
		,typeName.data,typeName.data);
	buffer_cpp.AddString( tmp );
	snprintf(tmp,MAX_NAME_SIZE,"\t\tif (res"
		,typeName.data);
	buffer_cpp.AddString( tmp );

	for(i=0;i<current_line;i++)
	{
		snprintf(tmp,MAX_NAME_SIZE,"&& \n\t\t O_T(%s)",lines[ i ].varName.data);
		buffer_cpp.AddString( tmp );
	}
	snprintf(tmp,MAX_NAME_SIZE,"\n\t\t) return true;\n\t\t else return false;");
	buffer_cpp.AddString( tmp );

	/////////
	//snprintf(tmp,MAX_NAME_SIZE,"\n ///////////////////////////////////////////////////////////////////////////////////////// \n");
	//buffer_cpp.AddString( tmp );
	/////////
	buffer_cpp.AddString( "\n\t}\n" );
	//buffer.AddString( "}\n" );
	*/
	return 1;
}
///////////////////////////////////////////////////////////////////////////////////////
inline int VS_AsnMetaTypeCompilerContainer::MHEADERClassMethods_choice(
								VS_AsnCompilerBuffer &buffer )
{	ZeroTmp();
	snprintf(tmp,MAX_NAME_SIZE,"\n \tenum{\n");
	buffer.AddString( tmp );
	unsigned long i=0;
	for(i=0;i<current_line;i++)
	{
		snprintf(tmp,MAX_NAME_SIZE,"\te_%s",lines[ i ].varName.data);
		buffer.AddString( tmp );
		if (i!=(current_line-1)) buffer.AddString(",\n");
		else buffer.AddString("\n");
	}
	snprintf(tmp,MAX_NAME_SIZE,"\t};\n");
	buffer.AddString( tmp );

	/////////////////////////////////////////////////

	snprintf(tmp,MAX_NAME_SIZE,"\tbool Decode( VS_PerBuffer &buffer );\n");
	buffer.AddString( tmp );

	/////////////////////////////////////////////////

	snprintf(tmp,MAX_NAME_SIZE,"\n\tvoid operator=(const %s & src);\n",typeName.data);
	buffer.AddString( tmp );

	/////////////////////////////////////////////////
	///ADD 29.09.2004 - pointer operators
	VS_AsnCompilerName tmpName;
	buffer.AddString( "\n");
	std::map<VS_AsnCompilerName,bool> DeclaredNames;
	for(i=0;i<current_line;i++)
	{
		if (lines[ i ].type==e_undefined)
		{
			if(DeclaredNames.end()!=DeclaredNames.find(lines[i].typeName))
				continue;
			snprintf(tmp,MAX_NAME_SIZE,"\t operator ");
			buffer.AddString( tmp );
			//
			VS_AsnCompilerName ttt;
			VS_AsnCompilerNameTranslator(lines[ i ].typeName , ttt);
			snprintf(tmp,MAX_NAME_SIZE,"%s *( void );\n",ttt.data);
			buffer.AddString( tmp );
			DeclaredNames[lines[i].typeName] = true;
		}
		else
		{
			//tmpName = lines[ i ].varName;
			//res = lines[ i ].MTypeName( buffer , tmpName, 0  );
			//if (1!=res) DError("MCPPClassMethods_choice - MType 2",res);
			//snprintf(tmp,MAX_NAME_SIZE," *( void );\n");
			//buffer.AddString( tmp );
		}

	}
	///ADD 29.09.2004 end
	/////////////////////////////////////////////////
	/// Show

	//snprintf(tmp,MAX_NAME_SIZE,"\n\tvoid Show( void );\n",typeName.data);
	snprintf(tmp,MAX_NAME_SIZE,"\n\tvoid Show( void );\n");
	buffer.AddString( tmp );

	/////////////////////////////////////////////////
	buffer.AddString("\n};\n");

	return 1;
}

inline int VS_AsnMetaTypeCompilerContainer::MCPPClassMethods_choice(
								VS_AsnCompilerBuffer &buffer_cpp )
{	ZeroTmp();
	VS_AsnCompilerName tmpName;
	/////////
	snprintf(tmp,MAX_NAME_SIZE,"\n ///////////////////////////////////////////////////////////////////////////////////////// \n");
	buffer_cpp.AddString( tmp );
	/////////
	//// DECODE

	snprintf(tmp,MAX_NAME_SIZE,"\tbool %s::Decode( VS_PerBuffer &buffer )\n\t{\n\t",typeName.data);
	buffer_cpp.AddString( tmp );
	snprintf(tmp,MAX_NAME_SIZE,"\tif (!buffer.ChoiceDecode(*this)) return false;\n\t");
	buffer_cpp.AddString( tmp );
	snprintf(tmp,MAX_NAME_SIZE,"\tswitch(tag)\n	\t{\n");
	buffer_cpp.AddString( tmp );
	unsigned long i=0;
	for(i=0;i<current_line;i++)
	{
		if (lines[ i ].type==e_undefined)
		{
			snprintf(tmp,MAX_NAME_SIZE,"\t\tcase e_%s : ",lines[ i ].varName.data);
			buffer_cpp.AddString( tmp );
			VS_AsnCompilerName ttt;
			VS_AsnCompilerNameTranslator(lines[ i ].typeName , ttt);
			snprintf(tmp,MAX_NAME_SIZE,"return DecodeChoice( buffer , new %s);\n",ttt.data);
			buffer_cpp.AddString( tmp );
		}
		else
		{
			snprintf(tmp,MAX_NAME_SIZE,"\t\tcase e_%s : ",lines[ i ].varName.data);
			buffer_cpp.AddString( tmp );

			VS_AsnCompilerName ttt;
			VS_AsnCompilerNameTranslator(lines[ i ].typeName , ttt);

			snprintf(tmp,MAX_NAME_SIZE,"return DecodeChoice( buffer , new ");// %s);\n",ttt.data;
			buffer_cpp.AddString( tmp );
			tmpName = lines[ i ].varName;
			res = lines[ i ].MTypeName( buffer_cpp , tmpName, 0  );
			if (1!=res) DError("MCPPClassMethods_choice - MType",res);
			snprintf(tmp,MAX_NAME_SIZE," );\n");
			buffer_cpp.AddString( tmp );
		}
	}
	if (isExtendable == true)
	{
			snprintf(tmp,MAX_NAME_SIZE,"\t\tdefault: return buffer.ChoiceMissExtensionObject(*this);");
			buffer_cpp.AddString( tmp );
	}
	else
	{
			snprintf(tmp,MAX_NAME_SIZE,"\t\tdefault: return false;");
			buffer_cpp.AddString( tmp );
	}
	buffer_cpp.AddString( "\n\t\t}\n" );
	buffer_cpp.AddString( "\n\t}\n" );
	/////////////////OPERATOR=
	snprintf(tmp,MAX_NAME_SIZE,"\n\tvoid %s::operator=(const %s & src)",typeName.data,typeName.data);
	buffer_cpp.AddString( tmp );
	snprintf(tmp,MAX_NAME_SIZE,"\n\t{\n\t\tFreeChoice();\n\t\tif (!src.filled) return;\n\t\tswitch(src.tag)\n\t\t{\n");
	buffer_cpp.AddString( tmp );
	for(i=0;i<current_line;i++)
	{
		if (lines[ i ].type==e_undefined)
		{
			snprintf(tmp,MAX_NAME_SIZE,"\t\tcase e_%s : ",lines[ i ].varName.data);
			buffer_cpp.AddString( tmp );
			VS_AsnCompilerName ttt;
			VS_AsnCompilerNameTranslator(lines[ i ].typeName , ttt);
			snprintf(tmp,MAX_NAME_SIZE,"CopyChoice< %s >(src,*this); return;\n",ttt.data);
			buffer_cpp.AddString( tmp );
		}
		else
		{
			snprintf(tmp,MAX_NAME_SIZE,"\t\tcase e_%s : ",lines[ i ].varName.data);
			buffer_cpp.AddString( tmp );
			snprintf(tmp,MAX_NAME_SIZE,"CopyChoice<");// %s >(src,*this);\n",string_types[ lines[ i ].type ]);
			buffer_cpp.AddString( tmp );
			tmpName = lines[ i ].varName;
			res = lines[ i ].MTypeName( buffer_cpp , tmpName, 0  );
			if (1!=res) DError("MCPPClassMethods_choice - MType 2",res);

			buffer_cpp.AddString( "  >(src,*this);  return;\n" );
		}
	}
	buffer_cpp.AddString( "\t\tdefault:		return;\n\t\t}\n" );
	buffer_cpp.AddString( "\n\t\treturn; \n\t}\n" );
	//////////////POINTER OPERATORS
	snprintf(tmp,MAX_NAME_SIZE,"\n ///////////////////////////////////////////////////////////////////////////////////////// \n");
	buffer_cpp.AddString( tmp );
	std::map<VS_AsnCompilerName,bool> DeclaredNames;
	for(i=0;i<current_line;i++)
	{
		if (lines[ i ].type==e_undefined)
		{
			if(DeclaredNames.end() != DeclaredNames.find(lines[i].typeName))
				continue;
			snprintf(tmp,MAX_NAME_SIZE,"\t%s::operator ",typeName.data);
			buffer_cpp.AddString( tmp );
			//
			VS_AsnCompilerName ttt;
			VS_AsnCompilerNameTranslator(lines[ i ].typeName , ttt);
			snprintf(tmp,MAX_NAME_SIZE,"%s *( void )\n\t{\t",ttt.data);
			buffer_cpp.AddString( tmp );
			snprintf(tmp,MAX_NAME_SIZE,"return dynamic_cast< %s ",ttt.data);
			buffer_cpp.AddString( tmp );
			//
			snprintf(tmp,MAX_NAME_SIZE,"* >(choice);    }\n\n ");
			buffer_cpp.AddString( tmp );
			DeclaredNames[lines[i].typeName] = true;
		}
		else
		{
			//tmpName = lines[ i ].varName;
			//res = lines[ i ].MTypeName( buffer_cpp , tmpName, 0  );
			//if (1!=res) DError("MCPPClassMethods_choice - MType 2",res);
			//snprintf(tmp,MAX_NAME_SIZE," *( void )\n\t{\t");
			//buffer_cpp.AddString( tmp );
			//snprintf(tmp,MAX_NAME_SIZE,"return dynamic_cast< ");
			//buffer_cpp.AddString( tmp );
			//tmpName = lines[ i ].varName;
			//res = lines[ i ].MTypeName( buffer_cpp , tmpName, 0  );
			//if (1!=res) DError("MCPPClassMethods_choice - MType 2",res);

		}
		//
	}
	////////////SHOW///////////////////
	snprintf(tmp,MAX_NAME_SIZE,"\n ///////////////////////////////////////////////////////////////////////////////////////// \n");
	buffer_cpp.AddString( tmp );

	snprintf(tmp,MAX_NAME_SIZE,"\tvoid %s::Show( void )\n\t{\n",typeName.data);
	buffer_cpp.AddString( tmp );
	snprintf(tmp,MAX_NAME_SIZE,"\t\tprintf(\"\\n\\t----------- %s::SHOW-----------\");\n\t\tif (!filled) return;\n",typeName.data);
	buffer_cpp.AddString( tmp );
	char c ='%';
	snprintf(tmp,MAX_NAME_SIZE,"\t\tprintf(\"Choice tag = %cd \",tag);\n",c);
	buffer_cpp.AddString( tmp );
	snprintf(tmp,MAX_NAME_SIZE,"\t\tswitch(tag)\n	\t{\n");
	buffer_cpp.AddString( tmp );
	for(i=0;i<current_line;i++)
	{
		if (lines[ i ].type==e_undefined)
		{
			snprintf(tmp,MAX_NAME_SIZE,"\t\tcase e_%s : ",lines[ i ].varName.data);
			buffer_cpp.AddString( tmp );
			VS_AsnCompilerName ttt;
			VS_AsnCompilerNameTranslator(lines[ i ].typeName , ttt);
			snprintf(tmp,MAX_NAME_SIZE," printf(\"\\n\\t choice: %s \");return;\n",ttt.data);
			buffer_cpp.AddString( tmp );
		}
		else
		{
			snprintf(tmp,MAX_NAME_SIZE,"\t\tcase e_%s : ",lines[ i ].varName.data);
			buffer_cpp.AddString( tmp );
			snprintf(tmp,MAX_NAME_SIZE," printf(\"\\n\\t choice: ");
			buffer_cpp.AddString( tmp );
			tmpName = lines[ i ].varName;
			res = lines[ i ].MTypeName( buffer_cpp , tmpName, 0  );
			if (1!=res) DError("MCPPClassMethods_choice - MType",res);
			snprintf(tmp,MAX_NAME_SIZE," \");return;\n");
			buffer_cpp.AddString( tmp );
		}
	}
	snprintf(tmp,MAX_NAME_SIZE,"\t\tdefault: printf(\"\\n\\t unknown choice: %cd\",tag); return ;",c);
	buffer_cpp.AddString( tmp );

	buffer_cpp.AddString( "\n\t\t}\n" );
	buffer_cpp.AddString( "\n\t}\n" );


	///////////////////////////////////

	return 1;
}
///////////////////////////////////////////////////////////////////////////////////////
inline int VS_AsnMetaTypeCompilerContainer::MHEADERClassMethods_strings(
								VS_AsnCompilerBuffer &buffer )
{	ZeroTmp();
	buffer.AddString("\n};");
	return 1;
}

inline int VS_AsnMetaTypeCompilerContainer::MCPPClassMethods_strings(
								VS_AsnCompilerBuffer &buffer_cpp )
{	ZeroTmp();
	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////





