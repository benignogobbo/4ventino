bool QventinoClient::dataDecode( std::string s ) {
  if( s[0] != 'D' && s[s.length()-2] != 'E' ) {
    return( false );
  }
  s = s.substr( 1 );
  _year = -9999;
  if( QventinoUtils::isInt( s.substr( 0, 4 ) ) ) {
    _year = std::stod( s.substr( 0, 4 ) );
  }
  s = s.substr( 4 );
  _month = -9999;
  if( QventinoUtils::isInt( s.substr( 0, 2 ) ) ) {
    _month = std::stod( s.substr( 0, 2 ) );
  }
  s = s.substr( 2 );
  _day = -9999;
  if( QventinoUtils::isInt( s.substr( 0, 2 ) ) ) {
    _day = std::stod( s.substr( 0, 2 ) );
  }
  s = s.substr( 2 );
  if( s[0] != 'T' ) {
    return( false );
  }
  s = s.substr( 1 );
  _hour = -9999;
  if( QventinoUtils::isInt( s.substr( 0, 2 ) ) ) {
    _hour = std::stod( s.substr( 0, 2 ) );
  }
  _minute = -9999;
  s = s.substr( 2 );
  if( QventinoUtils::isInt( s.substr( 0, 2 ) ) ) {
    _minute = std::stod( s.substr( 0, 2 ) );
  }
  s = s.substr( 2 );
  _second = -9999;
  if( QventinoUtils::isInt( s.substr( 0, 2 ) ) ) {
    _second = std::stod( s.substr( 0, 2 ) );
  }
  if( _year > -9999 && _month > -9999 && _day > -9999 && _hour > -9999 && _minute > -9999 && _second > -9999 ) {
    struct tm dataTime;
    dataTime.tm_year = _year - 1900;
    dataTime.tm_mon  = _month -1;
    dataTime.tm_mday = _day;
    dataTime.tm_hour = _hour;
    dataTime.tm_min  = _minute;
    dataTime.tm_sec  = _second;
    _sinceEpoch      = mktime( & dataTime );
  }
  else {
    _sinceEpoch = 0;
  }
  s = s.substr( 2 );
  for( int i=0; i<5; i++ ) {
    if( s[0] != 'M' ) {
      return( false ); 
    }
    s = s.substr( 1 );
    int pos = s.find( "M" );
    if( pos == -1 ) pos = s.length()-3;
    std::string meas = s.substr( 0, pos );
    if( QventinoUtils::isFloat( meas ) ) {
      _data[i] = std::stof( meas );
    }
    else {
      _data[i] = -9999;
    }
    s = s.substr( pos );
  }
  if( s[0] != 'E' ) {
    return( false ); 
  }
  return( true );
}

