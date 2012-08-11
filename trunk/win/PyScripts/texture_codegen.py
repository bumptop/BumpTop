# Copyright 2011 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

code='''		g_Texture[50] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B000002WQS.01._AA240_SCLZZZZZZZ_-1.tga").c_str());
		g_Texture[51] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B000002WYT.01._AA240_SCLZZZZZZZ_.tga").c_str());
		g_Texture[52] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B000003RY5.01._AA240_SCLZZZZZZZ_.tga").c_str());
		g_Texture[53] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B00004S51H.01._AA240_SCLZZZZZZZ_.tga").c_str());
		g_Texture[54] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B00004SU5J.01._AA240_SCLZZZZZZZ_.tga").c_str());
		g_Texture[55] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B000059MEK.01._AA180_SCLZZZZZZZ_.tga").c_str());
		g_Texture[56] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B00005U1YO.01._AA240_SCLZZZZZZZ_-1.tga").c_str());
		g_Texture[57] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B000068OSK.01._AA240_SCLZZZZZZZ_.tga").c_str());
		g_Texture[58] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B00006L88F.01._AA240_SCLZZZZZZZ_.tga").c_str());
		g_Texture[59] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B00009PJPR.01._AA240_SCLZZZZZZZ_.tga").c_str());
		g_Texture[60] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B0000DZFL0.01._AA240_SCLZZZZZZZ_.tga").c_str());
		g_Texture[61] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B0000E6EIZ.01._AA240_SCLZZZZZZZ_V51400228_.tga").c_str());
		g_Texture[62] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B00015HV4C.01._AA240_SCLZZZZZZZ_.tga").c_str());
		g_Texture[63] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B0001AP12G.01._AA240_SCLZZZZZZZ_V55972423_.tga").c_str());
		g_Texture[64] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B0001XARU4.01._AA240_SCLZZZZZZZ_.tga").c_str());
		g_Texture[65] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B000283OA8.01._AA240_SCLZZZZZZZ_.tga").c_str());
		g_Texture[66] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B0002IVN9W.01._AA240_SCLZZZZZZZ_.tga").c_str());
		g_Texture[67] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B0002VYQ4I.01._AA240_SCLZZZZZZZ_.tga").c_str());
		g_Texture[68] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B0002XL22U.01._AA240_SCLZZZZZZZ_.tga").c_str());
		g_Texture[69] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B00030EEO0.01._AA240_SCLZZZZZZZ_.tga").c_str());
		g_Texture[70] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B0006L16N8.01._AA240_SCLZZZZZZZ_.tga").c_str());
		g_Texture[71] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B0007KIFLO.01._AA240_SCLZZZZZZZ_.tga").c_str());
		g_Texture[72] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B0007NFMDK.01._AA240_SCLZZZZZZZ_.tga").c_str());
		g_Texture[73] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B0007PCDQ2.01._AA240_SCLZZZZZZZ_.tga").c_str());
		g_Texture[74] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B00082IJ08.01._AA240_SCLZZZZZZZ_-1.tga").c_str());
		g_Texture[75] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B0008KLVW8.01._AA240_SCLZZZZZZZ_.tga").c_str());
		g_Texture[76] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B0009FGWIK.01._SS500_SCLZZZZZZZ_V1115789175_.tga").c_str());
		g_Texture[77] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B0009WPKY0.01._AA240_SCLZZZZZZZ_.tga").c_str());
		g_Texture[78] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B000AOJHZA.01._AA240_SCLZZZZZZZ_.tga").c_str());
		g_Texture[79] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B000F39MEM.01._AA240_SCLZZZZZZZ_V63550020_.tga").c_str());
		g_Texture[80] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B000FPYNR6.01._AA240_SCLZZZZZZZ_V65902366_.tga").c_str());
		g_Texture[81] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B000G6BL7Y.01._AA240_SCLZZZZZZZ_V64199540_.tga").c_str());
		g_Texture[82] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\B000IHYBAU.01._AA240_SCLZZZZZZZ_V35699151_.tga").c_str());

'''

files=[]
while code.find('"') != -1:
    code=code[code.find('"',)+1:]
    files.append(code[:code.find('"')])
    code=code[code.find('"',)+1:]


startNum=50
for file in files:
    print 'g_Texture.push_back( ilutGLLoadImage((LPSTR)(textureDirectory + "' + file + '").c_str()) ); //' + str(startNum)
    startNum=startNum+1

    

filenames='''B000002WQS.01._AA240_SCLZZZZZZZ_-1.tga
B000002WYT.01._AA240_SCLZZZZZZZ_.tga
B000003RY5.01._AA240_SCLZZZZZZZ_.tga
B00004S51H.01._AA240_SCLZZZZZZZ_.tga
B00004SU5J.01._AA240_SCLZZZZZZZ_.tga
B000059MEK.01._AA180_SCLZZZZZZZ_.tga
B00005U1YO.01._AA240_SCLZZZZZZZ_-1.tga
B000068OSK.01._AA240_SCLZZZZZZZ_.tga
B00006L88F.01._AA240_SCLZZZZZZZ_.tga
B00009PJPR.01._AA240_SCLZZZZZZZ_.tga
B0000DZFL0.01._AA240_SCLZZZZZZZ_.tga
B0000E6EIZ.01._AA240_SCLZZZZZZZ_V51400228_.tga
B00015HV4C.01._AA240_SCLZZZZZZZ_.tga
B0001AP12G.01._AA240_SCLZZZZZZZ_V55972423_.tga
B0001XARU4.01._AA240_SCLZZZZZZZ_.tga
B000283OA8.01._AA240_SCLZZZZZZZ_.tga
B0002IVN9W.01._AA240_SCLZZZZZZZ_.tga
B0002VYQ4I.01._AA240_SCLZZZZZZZ_.tga
B0002XL22U.01._AA240_SCLZZZZZZZ_.tga
B00030EEO0.01._AA240_SCLZZZZZZZ_.tga
B0006L16N8.01._AA240_SCLZZZZZZZ_.tga
B0007KIFLO.01._AA240_SCLZZZZZZZ_.tga
B0007NFMDK.01._AA240_SCLZZZZZZZ_.tga
B0007PCDQ2.01._AA240_SCLZZZZZZZ_.tga
B00082IJ08.01._AA240_SCLZZZZZZZ_-1.tga
B0008KLVW8.01._AA240_SCLZZZZZZZ_.tga
B0009FGWIK.01._SS500_SCLZZZZZZZ_V1115789175_.tga
B0009WPKY0.01._AA240_SCLZZZZZZZ_.tga
B000AOJHZA.01._AA240_SCLZZZZZZZ_.tga
B000F39MEM.01._AA240_SCLZZZZZZZ_V63550020_.tga
B000FPYNR6.01._AA240_SCLZZZZZZZ_V65902366_.tga
B000G6BL7Y.01._AA240_SCLZZZZZZZ_V64199540_.tga
B000IHYBAU.01._AA240_SCLZZZZZZZ_V35699151_.tga'''

startNum=50

filenames=filenames.splitlines()

for file in filenames:
    #print 'CreateTexture(g_Texture, (LPSTR)(textureDirectory + "albums\\\\' + file + '").c_str(), ' + str(startNum) + ');' #old way, with CreateTexture()
    #print 'g_Texture[' + str(startNum) + '] = ilutGLLoadImage((LPSTR)(textureDirectory + "albums\\\\' + file + '").c_str());'
    startNum=startNum+1
