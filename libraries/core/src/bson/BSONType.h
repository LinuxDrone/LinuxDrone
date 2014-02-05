//--------------------------------------------------------------------
// This file was created as a part of the LinuxDrone project:
//                http://www.linuxdrone.org
//
// Distributed under the Creative Commons Attribution-ShareAlike 4.0
// International License (see accompanying License.txt file or a copy
// at http://creativecommons.org/licenses/by-sa/4.0/legalcode)
//
// The human-readable summary of (and not a substitute for) the
// license: http://creativecommons.org/licenses/by-sa/4.0/
//--------------------------------------------------------------------

#ifndef core_BSONType_h
#define core_BSONType_h

enum BSONType
{
    /** smaller than all other types */
    BSONType_MinKey=-1,
    /** end of object */
    BSONType_EOO=0,
    /** double precision floating point value */
    BSONType_NumberDouble=1,
    /** character string, stored in utf8 */
    BSONType_String=2,
    /** an embedded object */
    BSONType_Object=3,
    /** an embedded array */
    BSONType_Array=4,
    /** binary data */
    BSONType_BinData=5,
    /** Undefined type */
    BSONType_Undefined=6,
    /** ObjectId */
    BSONType_jstOID=7,
    /** boolean type */
    BSONType_Bool=8,
    /** date type */
    BSONType_Date=9,
    /** null type */
    BSONType_jstNULL=10,
    /** regular expression, a pattern with options */
    BSONType_RegEx=11,
    /** deprecated / will be redesigned */
    BSONType_DBRef=12,
    /** deprecated / use CodeWScope */
    BSONType_Code=13,
    /** a programming language (e.g., Python) symbol */
    BSONType_Symbol=14,
    /** javascript code that can execute on the database server, with SavedContext */
    BSONType_CodeWScope=15,
    /** 32 bit signed integer */
    BSONType_NumberInt = 16,
    /** Updated to a Date with value next OpTime on insert */
    BSONType_Timestamp = 17,
    /** 64 bit integer */
    BSONType_NumberLong = 18,
    /** max type that is not MaxKey */
    BSONType_JSTypeMax=18,
    /** larger than all other types */
    BSONType_MaxKey=127
};

#endif
