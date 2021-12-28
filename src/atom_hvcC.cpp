/*
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is MPEG4IP.
 *
 * The Initial Developer of the Original Code is Cisco Systems Inc.
 * Portions created by Cisco Systems Inc. are
 * Copyright (C) Cisco Systems Inc. 2004.  All Rights Reserved.
 *
 * Contributor(s):
 *      Bill May wmay@cisco.com
 */

#include "src/impl.h"

namespace mp4v2 {
namespace impl {

///////////////////////////////////////////////////////////////////////////////

MP4HvcCAtom::MP4HvcCAtom(MP4File &file)
        : MP4Atom(file, "hvcC")
{
    MP4TableProperty *pTable;

    AddProperty( new MP4Integer8Property(*this,"configurationVersion")); /* 0 */

    AddProperty( new MP4BitfieldProperty(*this,"generalProfileSpace", 2)); /* 1 */
    AddProperty( new MP4BitfieldProperty(*this,"generalTierFlag", 1)); /* 2 */
    AddProperty( new MP4BitfieldProperty(*this,"generalProfile", 5)); /* 3 */

    AddProperty( new MP4Integer32Property(*this,"generalProfileCompatibilityFlags")); /* 4 */

    AddProperty( new MP4Integer16Property(*this,"generalConstraintIndicatorFlagsHigh")); /* 5 */
    AddProperty( new MP4Integer32Property(*this,"generalConstraintIndicatorFlags")); /* 6 */

    AddProperty( new MP4Integer8Property(*this,"generalLevel")); /* 7 */

    AddProperty( new MP4BitfieldProperty(*this,"reserved1", 4)); /* 8 */
    AddProperty( new MP4BitfieldProperty(*this,"minSpatialSegmentation", 12)); /* 9 */

    AddProperty( new MP4BitfieldProperty(*this,"reserved2", 6)); /* 10 */
    AddProperty( new MP4BitfieldProperty(*this,"parallelismType", 2)); /* 11 */

    AddProperty( new MP4BitfieldProperty(*this,"reserved3", 6)); /* 12 */
    AddProperty( new MP4BitfieldProperty(*this,"chromaFormat", 2)); /* 13 */

    AddProperty( new MP4BitfieldProperty(*this,"reserved4", 5)); /* 14 */
    AddProperty( new MP4BitfieldProperty(*this,"lumaBitDepth", 3)); /* 15 */

    AddProperty( new MP4BitfieldProperty(*this,"reserved5", 5)); /* 16 */
    AddProperty( new MP4BitfieldProperty(*this,"chromaBitDepth", 3)); /* 17 */

    AddProperty( new MP4Integer16Property(*this,"averageFrameRate")); /* 18 */

    AddProperty( new MP4BitfieldProperty(*this,"constantFrameRate", 2)); /* 19 */
    AddProperty( new MP4BitfieldProperty(*this,"numTemporalLayers", 3)); /* 20 */
    AddProperty( new MP4BitfieldProperty(*this,"temporalIdNested", 1)); /* 21 */
    AddProperty( new MP4BitfieldProperty(*this,"lengthSizeMinusOne", 2)); /* 22 */

    AddProperty( new MP4Integer8Property(*this,"numOfSequences")); /* 23 */

    // vps
    AddProperty( new MP4Integer8Property(*this,"typeOfVideoParameterSets")); /* 24 */
    MP4Integer16Property *pCountVps = new MP4Integer16Property(*this,"numOfVideoParameterSets");
    AddProperty(pCountVps); /* 25 */

    pTable = new SizeTableProperty(*this,"videoEntries", pCountVps);
    AddProperty(pTable); /* 26 */
    pTable->AddProperty(new MP4Integer16Property(pTable->GetParentAtom(),"videoParameterSetLength"));
    pTable->AddProperty(new MP4BytesProperty(pTable->GetParentAtom(),"videoParameterSetNALUnit"));

    // sps
    AddProperty( new MP4Integer8Property(*this,"typeOfSequenceParameterSets")); /* 27 */
    MP4Integer16Property *pCountSps = new MP4Integer16Property(*this,"numOfSequenceParameterSets");
    AddProperty(pCountSps); /* 28 */

    pTable = new SizeTableProperty(*this,"sequenceEntries", pCountSps);
    AddProperty(pTable); /* 29 */
    pTable->AddProperty(new MP4Integer16Property(pTable->GetParentAtom(),"sequenceParameterSetLength"));
    pTable->AddProperty(new MP4BytesProperty(pTable->GetParentAtom(),"sequenceParameterSetNALUnit"));

    // pps
    AddProperty( new MP4Integer8Property(*this,"typeOfpictureParameterSets")); /* 30 */
    MP4Integer16Property *pCountPps = new MP4Integer16Property(*this,"numOfPictureParameterSets");
    AddProperty(pCountPps); /* 31 */

    pTable = new SizeTableProperty(*this,"pictureEntries", pCountPps);
    AddProperty(pTable); /* 32 */
    pTable->AddProperty(new MP4Integer16Property(pTable->GetParentAtom(),"pictureParameterSetLength"));
    pTable->AddProperty(new MP4BytesProperty(pTable->GetParentAtom(),"pictureParameterSetNALUnit"));
}

void MP4HvcCAtom::Generate()
{
    MP4Atom::Generate();

    ((MP4Integer8Property*)m_pProperties[0])->SetValue(1);

    m_pProperties[8]->SetReadOnly(false);
    ((MP4BitfieldProperty*)m_pProperties[8])->SetValue(0x0f);
    m_pProperties[8]->SetReadOnly(true);

    m_pProperties[10]->SetReadOnly(false);
    ((MP4BitfieldProperty*)m_pProperties[10])->SetValue(0x3f);
    m_pProperties[10]->SetReadOnly(true);

    m_pProperties[12]->SetReadOnly(false);
    ((MP4BitfieldProperty*)m_pProperties[12])->SetValue(0x3f);
    m_pProperties[14]->SetReadOnly(true);

    m_pProperties[14]->SetReadOnly(false);
    ((MP4BitfieldProperty*)m_pProperties[14])->SetValue(0x1f);
    m_pProperties[14]->SetReadOnly(true);

    m_pProperties[16]->SetReadOnly(false);
    ((MP4BitfieldProperty*)m_pProperties[16])->SetValue(0x1f);
    m_pProperties[16]->SetReadOnly(true);

#if 0
    // property reserved4 has non-zero fixed values
    static uint8_t reserved4[4] = {
        0x00, 0x18, 0xFF, 0xFF,
    };
    m_pProperties[7]->SetReadOnly(false);
    ((MP4BytesProperty*)m_pProperties[7])->
    SetValue(reserved4, sizeof(reserved4));
    m_pProperties[7]->SetReadOnly(true);
#endif
}

//
// Clone - clone my properties to destination atom
//
// this method simplifies duplicating avcC atom properties from
// source to destination file using a single API rather than
// having to copy each property.  This API encapsulates the object
// so the application layer need not concern with each property
// thereby isolating any future changes to atom properties.
//
// ----------------------------------------
// property   description
// ----------------------------------------
//
// 0    configurationVersion
// 1    AVCProfileIndication
// 2    profile_compatibility
// 3    AVCLevelIndication
// 4    reserved
// 5    lengthSizeMinusOne
// 6    reserved
// 7    number of SPS
// 8    SPS entries
// 9    number of PPS
// 10   PPS entries
//
//
void MP4HvcCAtom::Clone(MP4HvcCAtom *dstAtom)
{

    MP4Property *dstProperty;
    MP4TableProperty *pTable;
    uint16_t i16;
    uint64_t i32;
    uint64_t i64;
    uint8_t *tmp;

    // source pointer Property I16
    MP4Integer16Property *spPI16;
    // source pointer Property Bytes
    MP4BytesProperty *spPB;

    // dest pointer Property I16
    MP4Integer16Property *dpPI16;
    // dest pointer Property Bytes
    MP4BytesProperty *dpPB;


    // start with defaults and reserved fields
    dstAtom->Generate();

    // 0, 8, 10, 12, 14 16 are now generated from defaults
    // leaving others to export

    dstProperty=dstAtom->GetProperty(1);
    ((MP4BitfieldProperty *)dstProperty)->SetValue(
        ((MP4BitfieldProperty *)m_pProperties[1])->GetValue());
    dstProperty=dstAtom->GetProperty(2);
    ((MP4BitfieldProperty *)dstProperty)->SetValue(
        ((MP4BitfieldProperty *)m_pProperties[2])->GetValue());
    dstProperty=dstAtom->GetProperty(3);
    ((MP4BitfieldProperty *)dstProperty)->SetValue(
        ((MP4BitfieldProperty *)m_pProperties[3])->GetValue());

    dstProperty=dstAtom->GetProperty(4);
    ((MP4Integer32Property *)dstProperty)->SetValue(
        ((MP4Integer32Property *)m_pProperties[4])->GetValue());

    dstProperty=dstAtom->GetProperty(5);
    ((MP4Integer16Property *)dstProperty)->SetValue(
        ((MP4Integer16Property *)m_pProperties[5])->GetValue());

    dstProperty=dstAtom->GetProperty(6);
    ((MP4Integer32Property *)dstProperty)->SetValue(
        ((MP4Integer32Property *)m_pProperties[6])->GetValue());

    dstProperty=dstAtom->GetProperty(7);
    ((MP4Integer8Property *)dstProperty)->SetValue(
        ((MP4Integer8Property *)m_pProperties[7])->GetValue());

    dstProperty=dstAtom->GetProperty(9);
    ((MP4BitfieldProperty *)dstProperty)->SetValue(
        ((MP4BitfieldProperty *)m_pProperties[9])->GetValue());

    dstProperty=dstAtom->GetProperty(11);
    ((MP4BitfieldProperty *)dstProperty)->SetValue(
        ((MP4BitfieldProperty *)m_pProperties[11])->GetValue());

    dstProperty=dstAtom->GetProperty(13);
    ((MP4BitfieldProperty *)dstProperty)->SetValue(
        ((MP4BitfieldProperty *)m_pProperties[13])->GetValue());

    dstProperty=dstAtom->GetProperty(15);
    ((MP4BitfieldProperty *)dstProperty)->SetValue(
        ((MP4BitfieldProperty *)m_pProperties[15])->GetValue());

    dstProperty=dstAtom->GetProperty(17);
    ((MP4BitfieldProperty *)dstProperty)->SetValue(
        ((MP4BitfieldProperty *)m_pProperties[17])->GetValue());

    dstProperty=dstAtom->GetProperty(18);
    ((MP4Integer16Property *)dstProperty)->SetValue(
        ((MP4Integer16Property *)m_pProperties[18])->GetValue());

    dstProperty=dstAtom->GetProperty(19);
    ((MP4BitfieldProperty *)dstProperty)->SetValue(
        ((MP4BitfieldProperty *)m_pProperties[19])->GetValue());
    dstProperty=dstAtom->GetProperty(20);
    ((MP4BitfieldProperty *)dstProperty)->SetValue(
        ((MP4BitfieldProperty *)m_pProperties[20])->GetValue());
    dstProperty=dstAtom->GetProperty(21);
    ((MP4BitfieldProperty *)dstProperty)->SetValue(
        ((MP4BitfieldProperty *)m_pProperties[21])->GetValue());
    dstProperty=dstAtom->GetProperty(22);
    ((MP4BitfieldProperty *)dstProperty)->SetValue(
        ((MP4BitfieldProperty *)m_pProperties[22])->GetValue());

    dstProperty=dstAtom->GetProperty(23);
    ((MP4Integer8Property *)dstProperty)->SetValue(
        ((MP4Integer8Property *)m_pProperties[23])->GetValue());

    //
    // 24, 25 and 26 are related VPS (one set of sequence parameters)
    //

    // type
    dstProperty=dstAtom->GetProperty(24);
    ((MP4Integer8Property *)dstProperty)->SetValue(
        ((MP4Integer8Property *)m_pProperties[24])->GetValue());

    // first the count bitfield
    dstProperty=dstAtom->GetProperty(25);
    dstProperty->SetReadOnly(false);
    ((MP4Integer16Property *)dstProperty)->SetValue(
        ((MP4Integer16Property *)m_pProperties[25])->GetValue());
    dstProperty->SetReadOnly(true);

    // next export SPS Length and NAL bytes */

    // first source pointers
    pTable = (MP4TableProperty *) m_pProperties[26];
    spPI16 = (MP4Integer16Property *)pTable->GetProperty(0);
    spPB = (MP4BytesProperty *)pTable->GetProperty(1);

    // now dest pointers
    dstProperty=dstAtom->GetProperty(26);
    pTable = (MP4TableProperty *) dstProperty;
    dpPI16 = (MP4Integer16Property *)pTable->GetProperty(0);
    dpPB = (MP4BytesProperty *)pTable->GetProperty(1);

    // sps length
    i16 = spPI16->GetValue();
    i64 = i16;
    // FIXME - this leaves m_maxNumElements =2
    // but src atom m_maxNumElements is 1
    dpPI16->InsertValue(i64, 0);

    // export byte array
    i32 = i16;
    // copy bytes to local buffer
    tmp = (uint8_t *)MP4Malloc(i32);
    ASSERT(tmp != NULL);
    spPB->CopyValue(tmp, 0);
    // set element count
    dpPB->SetCount(1);
    // copy bytes
    dpPB->SetValue(tmp, i32, 0);
    MP4Free((void *)tmp);

    //
    // 27, 28 and 29 are related SPS (one set of sequence parameters)
    //

    // type
    dstProperty=dstAtom->GetProperty(27);
    ((MP4Integer8Property *)dstProperty)->SetValue(
        ((MP4Integer8Property *)m_pProperties[27])->GetValue());

    // first the count bitfield
    dstProperty=dstAtom->GetProperty(28);
    dstProperty->SetReadOnly(false);
    ((MP4Integer16Property *)dstProperty)->SetValue(
        ((MP4Integer16Property *)m_pProperties[28])->GetValue());
    dstProperty->SetReadOnly(true);

    // next export SPS Length and NAL bytes */

    // first source pointers
    pTable = (MP4TableProperty *) m_pProperties[29];
    spPI16 = (MP4Integer16Property *)pTable->GetProperty(0);
    spPB = (MP4BytesProperty *)pTable->GetProperty(1);

    // now dest pointers
    dstProperty=dstAtom->GetProperty(29);
    pTable = (MP4TableProperty *) dstProperty;
    dpPI16 = (MP4Integer16Property *)pTable->GetProperty(0);
    dpPB = (MP4BytesProperty *)pTable->GetProperty(1);

    // sps length
    i16 = spPI16->GetValue();
    i64 = i16;
    // FIXME - this leaves m_maxNumElements =2
    // but src atom m_maxNumElements is 1
    dpPI16->InsertValue(i64, 0);

    // export byte array
    i32 = i16;
    // copy bytes to local buffer
    tmp = (uint8_t *)MP4Malloc(i32);
    ASSERT(tmp != NULL);
    spPB->CopyValue(tmp, 0);
    // set element count
    dpPB->SetCount(1);
    // copy bytes
    dpPB->SetValue(tmp, i32, 0);
    MP4Free((void *)tmp);

    //
    // 30, 31 and 32 are related PPS (one set of picture parameters)
    //
    // first the integer8 count
    //
    // type
    dstProperty=dstAtom->GetProperty(30);
    ((MP4Integer8Property *)dstProperty)->SetValue(
        ((MP4Integer8Property *)m_pProperties[30])->GetValue());

    dstProperty=dstAtom->GetProperty(31);
    dstProperty->SetReadOnly(false);
    ((MP4Integer16Property *)dstProperty)->SetValue(
        ((MP4Integer16Property *)m_pProperties[31])->GetValue());
    dstProperty->SetReadOnly(true);

    // next export PPS Length and NAL bytes */

    // first source pointers
    pTable = (MP4TableProperty *) m_pProperties[32];
    spPI16 = (MP4Integer16Property *)pTable->GetProperty(0);
    spPB = (MP4BytesProperty *)pTable->GetProperty(1);

    // now dest pointers
    dstProperty=dstAtom->GetProperty(32);
    pTable = (MP4TableProperty *) dstProperty;
    dpPI16 = (MP4Integer16Property *)pTable->GetProperty(0);
    dpPB = (MP4BytesProperty *)pTable->GetProperty(1);

    // pps length
    i16 = spPI16->GetValue();
    i64 = i16;
    dpPI16->InsertValue(i64, 0);

    // export byte array
    i32 = i16;
    // copy bytes to local buffer
    tmp = (uint8_t *)MP4Malloc(i32);
    ASSERT(tmp != NULL);
    spPB->CopyValue(tmp, 0);
    // set element count
    dpPB->SetCount(1);
    // copy bytes
    dpPB->SetValue(tmp, i32, 0);
    MP4Free((void *)tmp);
}

///////////////////////////////////////////////////////////////////////////////

}
} // namespace mp4v2::impl
