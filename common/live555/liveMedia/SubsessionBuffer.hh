/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "liveMedia"
// Copyright (c) 1996-2016 Live Networks, Inc.  All rights reserved.

#ifndef _SUBSESSION_BUFFER_HH
#define _SUBSESSION_BUFFER_HH

#include "NetCommon.h"

class SubsessionBuffer {
public:
  SubsessionBuffer(unsigned bufferSize)
    : fBufferSize(bufferSize) {
    reset();
    fData = new unsigned char[bufferSize];
  }
  virtual ~SubsessionBuffer() { delete[] fData; }
  void reset() { fBytesInUse = 0; }
  void addBytes(unsigned numBytes) { fBytesInUse += numBytes; }

  unsigned char* dataStart() { return &fData[0]; }
  unsigned char* dataEnd() { return &fData[fBytesInUse]; }
  unsigned bytesInUse() const { return fBytesInUse; }
  unsigned bytesAvailable() const { return fBufferSize - fBytesInUse; }

  void setPresentationTime(struct timeval const& presentationTime) {
    fPresentationTime = presentationTime;
  }
  struct timeval const& presentationTime() const {return fPresentationTime;}

private:
  unsigned fBufferSize;
  struct timeval fPresentationTime;
  unsigned char* fData;
  unsigned fBytesInUse;
};
#endif
