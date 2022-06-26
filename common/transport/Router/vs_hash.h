////////////////////////////////////////////////////////////////
class VS_HashFunctionsIncapsulator
{
public:
    ////////////////////////////////////////////////////////////////
    unsigned __int64 FunctionKey(  const unsigned __int64 X,
                    const unsigned __int64 Y );

    ////////////////////////////////////////////////////////////////
    /// F(X+Y)
    unsigned __int64 FunctionXY(  const unsigned __int64 X,
                    const unsigned __int64 Y );

    ////////////////////////////////////////////////////////////////
    unsigned __int64 FunctionFXY( const unsigned __int64 X,
                    const unsigned __int64 Y );

    ////////////////////////////////////////////////////////////////
    unsigned __int64 MakeCheckSumm(  const unsigned char * buffer,
                                            const unsigned long size );
    ////////////////////////////////////////////////////////////////
    unsigned __int64 MakeCheckSummCRC32(  const unsigned char * buffer,
                                            const unsigned long size );

    ////////////////////////////////////////////////////////////////
    unsigned __int64 MakeHashAct( const unsigned char * buffer,
                                    const unsigned long size  );

    ////////////////////////////////////////////////////////////////
    unsigned __int64 MakeHash( const unsigned char * buffer,
                                    const unsigned long size  );

    ////////////////////////////////////////////////////////////////
    void ShowInt64(const unsigned __int64 word);

    ////////////////////////////////////////////////////////////////
    unsigned int DiffSequence( const unsigned char * buff,
                            const unsigned char * new_buffer,
                        const unsigned long size);

    ////////////////////////////////////////////////////////////////
    void ShowSequence(const unsigned char * word,
                    const unsigned long size);

    ////////////////////////////////////////////////////////////////
    void ChangeSequenceSubsets( const unsigned char * buff,
                            unsigned char *& new_buffer,
                        const unsigned long size,
                        const unsigned long times );

    ////////////////////////////////////////////////////////////////
    void ChangeSequence( const unsigned char * buff,
                            unsigned char *& new_buffer,
                        const unsigned long size,
                        const unsigned long times );

    ////////////////////////////////////////////////////////////////
    void GenerateSequence( unsigned char *& buff,
                        const unsigned long size);
    ////////////////////////////////////////////////////////////////
    unsigned long GetNeededSize(const unsigned long size);

    ////////////////////////////////////////////////////////////////
    unsigned __int64 MakeHashWithoutAlloc(
                                const unsigned char * buffer,
                                unsigned char * new_buffer,
                                const unsigned long size  );
    ////////////////////////////////////////////////////////////////
};