#pragma once

class cstring {
public:
    cstring( const char* data, int len = -1 ) {
        assign( data, len );
    }
    cstring( const cstring& s ) {
        if ( &s==this ) return;
        assign( s._data, s._len );
    }
    ~cstring() {
        if ( _data!=nullptr ) ::free( _data );
        _len = 0;
        _data = nullptr;
    }
    void assign( const char* data, int len = -1 ) {
        if ( len<0 ) len = ::strlen( data );
        _data = ::strndup( data, len );
        _len = len;
    }
private:
    char*    _data;
    uint32_t _len;
};
