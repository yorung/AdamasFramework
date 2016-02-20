xof 0302txt 0064
template Header {
 <3D82AB43-62DA-11cf-AB39-0020AF71E433>
 WORD major;
 WORD minor;
 DWORD flags;
}

template Vector {
 <3D82AB5E-62DA-11cf-AB39-0020AF71E433>
 FLOAT x;
 FLOAT y;
 FLOAT z;
}

template Coords2d {
 <F6F23F44-7686-11cf-8F52-0040333594A3>
 FLOAT u;
 FLOAT v;
}

template Matrix4x4 {
 <F6F23F45-7686-11cf-8F52-0040333594A3>
 array FLOAT matrix[16];
}

template ColorRGBA {
 <35FF44E0-6C7C-11cf-8F52-0040333594A3>
 FLOAT red;
 FLOAT green;
 FLOAT blue;
 FLOAT alpha;
}

template ColorRGB {
 <D3E16E81-7835-11cf-8F52-0040333594A3>
 FLOAT red;
 FLOAT green;
 FLOAT blue;
}

template IndexedColor {
 <1630B820-7842-11cf-8F52-0040333594A3>
 DWORD index;
 ColorRGBA indexColor;
}

template Boolean {
 <4885AE61-78E8-11cf-8F52-0040333594A3>
 WORD truefalse;
}

template Boolean2d {
 <4885AE63-78E8-11cf-8F52-0040333594A3>
 Boolean u;
 Boolean v;
}

template MaterialWrap {
 <4885AE60-78E8-11cf-8F52-0040333594A3>
 Boolean u;
 Boolean v;
}

template TextureFilename {
 <A42790E1-7810-11cf-8F52-0040333594A3>
 STRING filename;
}

template Material {
 <3D82AB4D-62DA-11cf-AB39-0020AF71E433>
 ColorRGBA faceColor;
 FLOAT power;
 ColorRGB specularColor;
 ColorRGB emissiveColor;
 [...]
}

template MeshFace {
 <3D82AB5F-62DA-11cf-AB39-0020AF71E433>
 DWORD nFaceVertexIndices;
 array DWORD faceVertexIndices[nFaceVertexIndices];
}

template MeshFaceWraps {
 <4885AE62-78E8-11cf-8F52-0040333594A3>
 DWORD nFaceWrapValues;
 Boolean2d faceWrapValues;
}

template MeshTextureCoords {
 <F6F23F40-7686-11cf-8F52-0040333594A3>
 DWORD nTextureCoords;
 array Coords2d textureCoords[nTextureCoords];
}

template MeshMaterialList {
 <F6F23F42-7686-11cf-8F52-0040333594A3>
 DWORD nMaterials;
 DWORD nFaceIndexes;
 array DWORD faceIndexes[nFaceIndexes];
 [Material]
}

template MeshNormals {
 <F6F23F43-7686-11cf-8F52-0040333594A3>
 DWORD nNormals;
 array Vector normals[nNormals];
 DWORD nFaceNormals;
 array MeshFace faceNormals[nFaceNormals];
}

template MeshVertexColors {
 <1630B821-7842-11cf-8F52-0040333594A3>
 DWORD nVertexColors;
 array IndexedColor vertexColors[nVertexColors];
}

template Mesh {
 <3D82AB44-62DA-11cf-AB39-0020AF71E433>
 DWORD nVertices;
 array Vector vertices[nVertices];
 DWORD nFaces;
 array MeshFace faces[nFaces];
 [...]
}

Header{
1;
0;
1;
}

Mesh {
 105;
 0.00000;1.00000;0.00000;,
 0.27060;0.92388;-0.27060;,
 0.00000;0.92388;-0.38268;,
 0.00000;1.00000;0.00000;,
 0.38268;0.92388;0.00000;,
 0.00000;1.00000;0.00000;,
 0.27060;0.92388;0.27060;,
 0.00000;1.00000;0.00000;,
 -0.00000;0.92388;0.38268;,
 0.00000;1.00000;0.00000;,
 -0.27060;0.92388;0.27060;,
 0.00000;1.00000;0.00000;,
 -0.38268;0.92388;-0.00000;,
 0.00000;1.00000;0.00000;,
 -0.27060;0.92388;-0.27060;,
 0.00000;1.00000;0.00000;,
 0.00000;0.92388;-0.38268;,
 0.50000;0.70711;-0.50000;,
 0.00000;0.70711;-0.70711;,
 0.70711;0.70711;0.00000;,
 0.50000;0.70711;0.50000;,
 -0.00000;0.70711;0.70711;,
 -0.50000;0.70711;0.50000;,
 -0.70711;0.70711;-0.00000;,
 -0.50000;0.70711;-0.50000;,
 0.00000;0.70711;-0.70711;,
 0.65328;0.38268;-0.65328;,
 0.00000;0.38268;-0.92388;,
 0.92388;0.38268;0.00000;,
 0.65328;0.38268;0.65328;,
 -0.00000;0.38268;0.92388;,
 -0.65328;0.38268;0.65328;,
 -0.92388;0.38268;-0.00000;,
 -0.65328;0.38268;-0.65328;,
 0.00000;0.38268;-0.92388;,
 0.70711;-0.00000;-0.70711;,
 0.00000;-0.00000;-2.71134;,
 3.44095;-0.00000;0.00000;,
 0.70711;-0.00000;0.70711;,
 -0.00000;-0.00000;3.20636;,
 -0.70711;-0.00000;0.70711;,
 -3.24561;-0.00000;-0.00000;,
 -0.70711;-0.00000;-0.70711;,
 0.00000;-0.00000;-2.71134;,
 0.65328;-0.38268;-0.65328;,
 0.00000;-0.38268;-0.92388;,
 0.92388;-0.38268;0.00000;,
 0.65328;-0.38268;0.65328;,
 -0.00000;-0.38268;0.92388;,
 -0.65328;-0.38268;0.65328;,
 -0.92388;-0.38268;-0.00000;,
 -0.65328;-0.38268;-0.65328;,
 0.00000;-0.38268;-0.92388;,
 0.50000;-0.70711;-0.50000;,
 0.00000;-0.70711;-0.70711;,
 0.70711;-0.70711;0.00000;,
 0.50000;-0.70711;0.50000;,
 -0.00000;-0.70711;0.70711;,
 -0.50000;-0.70711;0.50000;,
 -0.70711;-0.70711;-0.00000;,
 -0.50000;-0.70711;-0.50000;,
 0.00000;-0.70711;-0.70711;,
 0.27060;-0.92388;-0.27060;,
 0.00000;-0.92388;-0.38268;,
 0.38268;-0.92388;0.00000;,
 0.27060;-0.92388;0.27060;,
 -0.00000;-0.92388;0.38268;,
 -0.27060;-0.92388;0.27060;,
 -0.38268;-0.92388;-0.00000;,
 -0.27060;-0.92388;-0.27060;,
 0.00000;-0.92388;-0.38268;,
 0.00000;-2.51031;-0.00000;,
 0.00000;-2.51031;-0.00000;,
 0.00000;-2.51031;-0.00000;,
 0.00000;-2.51031;-0.00000;,
 0.00000;-2.51031;-0.00000;,
 0.00000;-2.51031;-0.00000;,
 0.00000;-2.51031;-0.00000;,
 0.00000;-2.51031;-0.00000;,
 0.00000;0.00000;-0.50000;,
 0.00000;2.00000;0.00000;,
 0.35355;0.00000;-0.35355;,
 0.00000;2.00000;0.00000;,
 0.50000;0.00000;0.00000;,
 0.00000;2.00000;0.00000;,
 0.35355;0.00000;0.35355;,
 0.00000;2.00000;0.00000;,
 -0.00000;0.00000;0.50000;,
 0.00000;2.00000;0.00000;,
 -0.35355;0.00000;0.35355;,
 0.00000;2.00000;0.00000;,
 -0.50000;0.00000;-0.00000;,
 0.00000;2.00000;0.00000;,
 -0.35355;0.00000;-0.35355;,
 0.00000;2.00000;0.00000;,
 0.00000;0.00000;-0.50000;,
 0.00000;0.00000;0.00000;,
 0.00000;0.00000;-0.50000;,
 0.35355;0.00000;-0.35355;,
 0.50000;0.00000;0.00000;,
 0.35355;0.00000;0.35355;,
 -0.00000;0.00000;0.50000;,
 -0.35355;0.00000;0.35355;,
 -0.50000;0.00000;-0.00000;,
 -0.35355;0.00000;-0.35355;;
 
 80;
 3;0,1,2;,
 3;3,4,1;,
 3;5,6,4;,
 3;7,8,6;,
 3;9,10,8;,
 3;11,12,10;,
 3;13,14,12;,
 3;15,16,14;,
 4;2,1,17,18;,
 4;1,4,19,17;,
 4;4,6,20,19;,
 4;6,8,21,20;,
 4;8,10,22,21;,
 4;10,12,23,22;,
 4;12,14,24,23;,
 4;14,16,25,24;,
 4;18,17,26,27;,
 4;17,19,28,26;,
 4;19,20,29,28;,
 4;20,21,30,29;,
 4;21,22,31,30;,
 4;22,23,32,31;,
 4;23,24,33,32;,
 4;24,25,34,33;,
 4;27,26,35,36;,
 4;26,28,37,35;,
 4;28,29,38,37;,
 4;29,30,39,38;,
 4;30,31,40,39;,
 4;31,32,41,40;,
 4;32,33,42,41;,
 4;33,34,43,42;,
 4;36,35,44,45;,
 4;35,37,46,44;,
 4;37,38,47,46;,
 4;38,39,48,47;,
 4;39,40,49,48;,
 4;40,41,50,49;,
 4;41,42,51,50;,
 4;42,43,52,51;,
 4;45,44,53,54;,
 4;44,46,55,53;,
 4;46,47,56,55;,
 4;47,48,57,56;,
 4;48,49,58,57;,
 4;49,50,59,58;,
 4;50,51,60,59;,
 4;51,52,61,60;,
 4;54,53,62,63;,
 4;53,55,64,62;,
 4;55,56,65,64;,
 4;56,57,66,65;,
 4;57,58,67,66;,
 4;58,59,68,67;,
 4;59,60,69,68;,
 4;60,61,70,69;,
 3;63,62,71;,
 3;62,64,72;,
 3;64,65,73;,
 3;65,66,74;,
 3;66,67,75;,
 3;67,68,76;,
 3;68,69,77;,
 3;69,70,78;,
 3;79,80,81;,
 3;81,82,83;,
 3;83,84,85;,
 3;85,86,87;,
 3;87,88,89;,
 3;89,90,91;,
 3;91,92,93;,
 3;93,94,95;,
 3;96,97,98;,
 3;96,98,99;,
 3;96,99,100;,
 3;96,100,101;,
 3;96,101,102;,
 3;96,102,103;,
 3;96,103,104;,
 3;96,104,97;;
 
 MeshMaterialList {
  1;
  80;
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0;;
  Material {
   1.000000;1.000000;1.000000;1.000000;;
   0.000000;
   0.000000;0.000000;0.000000;;
   0.000000;0.000000;0.000000;;
  }
 }
 MeshNormals {
  86;
  -0.000000;1.000000;0.000000;,
  -0.000000;0.924735;-0.380611;,
  0.269133;0.924735;-0.269132;,
  0.380611;0.924735;0.000000;,
  0.269132;0.924735;0.269132;,
  -0.000000;0.924735;0.380611;,
  -0.269132;0.924735;0.269132;,
  -0.380611;0.924735;0.000000;,
  -0.269132;0.924735;-0.269132;,
  -0.000000;0.709230;-0.704977;,
  0.498494;0.709230;-0.498494;,
  0.704977;0.709230;0.000000;,
  0.498494;0.709230;0.498494;,
  -0.000000;0.709230;0.704977;,
  -0.498494;0.709230;0.498494;,
  -0.704977;0.709230;0.000000;,
  -0.498494;0.709230;-0.498494;,
  -0.000000;0.618973;-0.785412;,
  0.574118;0.577909;-0.580009;,
  0.776487;0.630133;0.000000;,
  0.574689;0.581150;0.576192;,
  -0.000000;0.627298;0.778779;,
  -0.575762;0.580245;0.576033;,
  -0.778366;0.627811;0.000000;,
  -0.575189;0.577003;-0.579849;,
  0.510626;0.588368;-0.626964;,
  0.560615;0.598688;-0.572087;,
  0.607454;0.605646;-0.513996;,
  0.561620;0.604872;0.564546;,
  0.513171;0.601282;0.612467;,
  -0.563743;0.603150;0.564272;,
  -0.611562;0.602073;0.513322;,
  -0.562735;0.596961;-0.571810;,
  0.183025;-0.587453;-0.788290;,
  0.574118;-0.577909;-0.580008;,
  0.782557;-0.594603;-0.184530;,
  0.574689;-0.581150;0.576192;,
  0.184156;-0.592785;0.784023;,
  -0.575762;-0.580245;0.576033;,
  -0.783759;-0.593114;0.184224;,
  -0.575189;-0.577003;-0.579849;,
  -0.000000;-0.709230;-0.704977;,
  0.498494;-0.709230;-0.498494;,
  0.704977;-0.709230;0.000000;,
  0.498494;-0.709230;0.498494;,
  -0.000000;-0.709230;0.704977;,
  -0.498494;-0.709230;0.498494;,
  -0.704977;-0.709230;-0.000000;,
  -0.498494;-0.709230;-0.498494;,
  -0.000000;-0.580107;-0.814540;,
  0.575967;-0.580107;-0.575967;,
  0.814540;-0.580107;0.000000;,
  0.575967;-0.580107;0.575967;,
  -0.000000;-0.580107;0.814540;,
  -0.575967;-0.580107;0.575967;,
  -0.814540;-0.580107;-0.000000;,
  -0.575967;-0.580107;-0.575967;,
  0.687391;-0.234496;-0.687391;,
  -0.000000;0.242536;-0.970142;,
  0.685994;0.242536;-0.685994;,
  0.970143;0.242536;0.000000;,
  0.685994;0.242536;0.685994;,
  -0.000000;0.242536;0.970143;,
  -0.685994;0.242536;0.685994;,
  -0.970143;0.242536;-0.000000;,
  -0.685994;0.242536;-0.685994;,
  0.000000;-1.000000;-0.000000;,
  0.607454;0.605646;0.513996;,
  -0.513171;0.601282;0.612466;,
  -0.611562;0.602073;-0.513322;,
  -0.510626;0.588368;-0.626964;,
  0.510626;-0.588368;-0.626964;,
  0.560615;-0.598688;-0.572087;,
  0.607454;-0.605646;-0.513996;,
  0.607454;-0.605646;0.513996;,
  0.561620;-0.604872;0.564546;,
  0.513171;-0.601282;0.612467;,
  -0.513171;-0.601282;0.612466;,
  -0.563743;-0.603150;0.564272;,
  -0.611562;-0.602073;0.513322;,
  -0.611562;-0.602073;-0.513322;,
  -0.562735;-0.596961;-0.571810;,
  -0.510626;-0.588368;-0.626964;,
  0.687390;-0.234496;0.687391;,
  -0.687391;-0.234496;0.687390;,
  -0.687391;-0.234496;-0.687390;;
  80;
  3;0,2,1;,
  3;0,3,2;,
  3;0,4,3;,
  3;0,5,4;,
  3;0,6,5;,
  3;0,7,6;,
  3;0,8,7;,
  3;0,1,8;,
  4;1,2,10,9;,
  4;2,3,11,10;,
  4;3,4,12,11;,
  4;4,5,13,12;,
  4;5,6,14,13;,
  4;6,7,15,14;,
  4;7,8,16,15;,
  4;8,1,9,16;,
  4;9,10,18,17;,
  4;10,11,19,18;,
  4;11,12,20,19;,
  4;12,13,21,20;,
  4;13,14,22,21;,
  4;14,15,23,22;,
  4;15,16,24,23;,
  4;16,9,17,24;,
  4;17,18,26,25;,
  4;18,19,27,26;,
  4;19,20,28,67;,
  4;20,21,29,28;,
  4;21,22,30,68;,
  4;22,23,31,30;,
  4;23,24,32,69;,
  4;24,17,70,32;,
  4;71,72,34,33;,
  4;72,73,35,34;,
  4;74,75,36,74;,
  4;75,76,37,36;,
  4;77,78,38,77;,
  4;78,79,39,38;,
  4;80,81,40,80;,
  4;81,82,82,40;,
  4;33,34,42,41;,
  4;34,35,43,42;,
  4;35,36,44,43;,
  4;36,37,45,44;,
  4;37,38,46,45;,
  4;38,39,47,46;,
  4;39,40,48,47;,
  4;40,33,41,48;,
  4;41,42,50,49;,
  4;42,43,51,50;,
  4;43,44,52,51;,
  4;44,45,53,52;,
  4;45,46,54,53;,
  4;46,47,55,54;,
  4;47,48,56,55;,
  4;48,41,49,56;,
  3;49,50,57;,
  3;50,51,57;,
  3;51,52,83;,
  3;52,53,83;,
  3;53,54,84;,
  3;54,55,84;,
  3;55,56,85;,
  3;56,49,85;,
  3;58,59,59;,
  3;59,59,60;,
  3;60,61,61;,
  3;61,61,62;,
  3;62,63,63;,
  3;63,63,64;,
  3;64,65,65;,
  3;65,65,58;,
  3;66,66,66;,
  3;66,66,66;,
  3;66,66,66;,
  3;66,66,66;,
  3;66,66,66;,
  3;66,66,66;,
  3;66,66,66;,
  3;66,66,66;;
 }
 MeshTextureCoords {
  105;
  0.062500;0.000000;,
  0.125000;0.125000;,
  0.000000;0.125000;,
  0.187500;0.000000;,
  0.250000;0.125000;,
  0.312500;0.000000;,
  0.375000;0.125000;,
  0.437500;0.000000;,
  0.500000;0.125000;,
  0.562500;0.000000;,
  0.625000;0.125000;,
  0.687500;0.000000;,
  0.750000;0.125000;,
  0.812500;0.000000;,
  0.875000;0.125000;,
  0.937500;0.000000;,
  1.000000;0.125000;,
  0.125000;0.250000;,
  0.000000;0.250000;,
  0.250000;0.250000;,
  0.375000;0.250000;,
  0.500000;0.250000;,
  0.625000;0.250000;,
  0.750000;0.250000;,
  0.875000;0.250000;,
  1.000000;0.250000;,
  0.125000;0.375000;,
  0.000000;0.375000;,
  0.250000;0.375000;,
  0.375000;0.375000;,
  0.500000;0.375000;,
  0.625000;0.375000;,
  0.750000;0.375000;,
  0.875000;0.375000;,
  1.000000;0.375000;,
  0.125000;0.500000;,
  0.000000;0.500000;,
  0.250000;0.500000;,
  0.375000;0.500000;,
  0.500000;0.500000;,
  0.625000;0.500000;,
  0.750000;0.500000;,
  0.875000;0.500000;,
  1.000000;0.500000;,
  0.125000;0.625000;,
  0.000000;0.625000;,
  0.250000;0.625000;,
  0.375000;0.625000;,
  0.500000;0.625000;,
  0.625000;0.625000;,
  0.750000;0.625000;,
  0.875000;0.625000;,
  1.000000;0.625000;,
  0.125000;0.750000;,
  0.000000;0.750000;,
  0.250000;0.750000;,
  0.375000;0.750000;,
  0.500000;0.750000;,
  0.625000;0.750000;,
  0.750000;0.750000;,
  0.875000;0.750000;,
  1.000000;0.750000;,
  0.125000;0.875000;,
  0.000000;0.875000;,
  0.250000;0.875000;,
  0.375000;0.875000;,
  0.500000;0.875000;,
  0.625000;0.875000;,
  0.750000;0.875000;,
  0.875000;0.875000;,
  1.000000;0.875000;,
  0.062500;1.000000;,
  0.187500;1.000000;,
  0.312500;1.000000;,
  0.437500;1.000000;,
  0.562500;1.000000;,
  0.687500;1.000000;,
  0.812500;1.000000;,
  0.937500;1.000000;,
  0.000000;1.000000;,
  0.062500;0.000000;,
  0.125000;1.000000;,
  0.125000;0.000000;,
  0.250000;1.000000;,
  0.187500;0.000000;,
  0.375000;1.000000;,
  0.250000;0.000000;,
  0.500000;1.000000;,
  0.312500;0.000000;,
  0.625000;1.000000;,
  0.375000;0.000000;,
  0.750000;1.000000;,
  0.437500;0.000000;,
  0.875000;1.000000;,
  0.500000;0.000000;,
  1.000000;1.000000;,
  0.000000;0.000000;,
  0.000000;0.000000;,
  0.000000;0.000000;,
  0.000000;0.000000;,
  0.000000;0.000000;,
  0.000000;0.000000;,
  0.000000;0.000000;,
  0.000000;0.000000;,
  0.000000;0.000000;;
 }
 MeshVertexColors {
  105;
  0;1.000000;1.000000;1.000000;1.000000;,
  1;1.000000;1.000000;1.000000;1.000000;,
  2;1.000000;1.000000;1.000000;1.000000;,
  3;1.000000;1.000000;1.000000;1.000000;,
  4;1.000000;1.000000;1.000000;1.000000;,
  5;1.000000;1.000000;1.000000;1.000000;,
  6;1.000000;1.000000;1.000000;1.000000;,
  7;1.000000;1.000000;1.000000;1.000000;,
  8;1.000000;1.000000;1.000000;1.000000;,
  9;1.000000;1.000000;1.000000;1.000000;,
  10;1.000000;1.000000;1.000000;1.000000;,
  11;1.000000;1.000000;1.000000;1.000000;,
  12;1.000000;1.000000;1.000000;1.000000;,
  13;1.000000;1.000000;1.000000;1.000000;,
  14;1.000000;1.000000;1.000000;1.000000;,
  15;1.000000;1.000000;1.000000;1.000000;,
  16;1.000000;1.000000;1.000000;1.000000;,
  17;1.000000;1.000000;1.000000;1.000000;,
  18;1.000000;1.000000;1.000000;1.000000;,
  19;1.000000;1.000000;1.000000;1.000000;,
  20;1.000000;1.000000;1.000000;1.000000;,
  21;1.000000;1.000000;1.000000;1.000000;,
  22;1.000000;1.000000;1.000000;1.000000;,
  23;1.000000;1.000000;1.000000;1.000000;,
  24;1.000000;1.000000;1.000000;1.000000;,
  25;1.000000;1.000000;1.000000;1.000000;,
  26;1.000000;1.000000;1.000000;1.000000;,
  27;1.000000;1.000000;1.000000;1.000000;,
  28;1.000000;1.000000;1.000000;1.000000;,
  29;1.000000;1.000000;1.000000;1.000000;,
  30;1.000000;1.000000;1.000000;1.000000;,
  31;1.000000;1.000000;1.000000;1.000000;,
  32;1.000000;1.000000;1.000000;1.000000;,
  33;1.000000;1.000000;1.000000;1.000000;,
  34;1.000000;1.000000;1.000000;1.000000;,
  35;1.000000;1.000000;1.000000;1.000000;,
  36;1.000000;1.000000;1.000000;1.000000;,
  37;1.000000;1.000000;1.000000;1.000000;,
  38;1.000000;1.000000;1.000000;1.000000;,
  39;1.000000;1.000000;1.000000;1.000000;,
  40;1.000000;1.000000;1.000000;1.000000;,
  41;1.000000;1.000000;1.000000;1.000000;,
  42;1.000000;1.000000;1.000000;1.000000;,
  43;1.000000;1.000000;1.000000;1.000000;,
  44;1.000000;1.000000;1.000000;1.000000;,
  45;1.000000;1.000000;1.000000;1.000000;,
  46;1.000000;1.000000;1.000000;1.000000;,
  47;1.000000;1.000000;1.000000;1.000000;,
  48;1.000000;1.000000;1.000000;1.000000;,
  49;1.000000;1.000000;1.000000;1.000000;,
  50;1.000000;1.000000;1.000000;1.000000;,
  51;1.000000;1.000000;1.000000;1.000000;,
  52;1.000000;1.000000;1.000000;1.000000;,
  53;1.000000;1.000000;1.000000;1.000000;,
  54;1.000000;1.000000;1.000000;1.000000;,
  55;1.000000;1.000000;1.000000;1.000000;,
  56;1.000000;1.000000;1.000000;1.000000;,
  57;1.000000;1.000000;1.000000;1.000000;,
  58;1.000000;1.000000;1.000000;1.000000;,
  59;1.000000;1.000000;1.000000;1.000000;,
  60;1.000000;1.000000;1.000000;1.000000;,
  61;1.000000;1.000000;1.000000;1.000000;,
  62;1.000000;1.000000;1.000000;1.000000;,
  63;1.000000;1.000000;1.000000;1.000000;,
  64;1.000000;1.000000;1.000000;1.000000;,
  65;1.000000;1.000000;1.000000;1.000000;,
  66;1.000000;1.000000;1.000000;1.000000;,
  67;1.000000;1.000000;1.000000;1.000000;,
  68;1.000000;1.000000;1.000000;1.000000;,
  69;1.000000;1.000000;1.000000;1.000000;,
  70;1.000000;1.000000;1.000000;1.000000;,
  71;1.000000;1.000000;1.000000;1.000000;,
  72;1.000000;1.000000;1.000000;1.000000;,
  73;1.000000;1.000000;1.000000;1.000000;,
  74;1.000000;1.000000;1.000000;1.000000;,
  75;1.000000;1.000000;1.000000;1.000000;,
  76;1.000000;1.000000;1.000000;1.000000;,
  77;1.000000;1.000000;1.000000;1.000000;,
  78;1.000000;1.000000;1.000000;1.000000;,
  79;1.000000;1.000000;1.000000;1.000000;,
  80;1.000000;1.000000;1.000000;1.000000;,
  81;1.000000;1.000000;1.000000;1.000000;,
  82;1.000000;1.000000;1.000000;1.000000;,
  83;1.000000;1.000000;1.000000;1.000000;,
  84;1.000000;1.000000;1.000000;1.000000;,
  85;1.000000;1.000000;1.000000;1.000000;,
  86;1.000000;1.000000;1.000000;1.000000;,
  87;1.000000;1.000000;1.000000;1.000000;,
  88;1.000000;1.000000;1.000000;1.000000;,
  89;1.000000;1.000000;1.000000;1.000000;,
  90;1.000000;1.000000;1.000000;1.000000;,
  91;1.000000;1.000000;1.000000;1.000000;,
  92;1.000000;1.000000;1.000000;1.000000;,
  93;1.000000;1.000000;1.000000;1.000000;,
  94;1.000000;1.000000;1.000000;1.000000;,
  95;1.000000;1.000000;1.000000;1.000000;,
  96;1.000000;1.000000;1.000000;1.000000;,
  97;1.000000;1.000000;1.000000;1.000000;,
  98;1.000000;1.000000;1.000000;1.000000;,
  99;1.000000;1.000000;1.000000;1.000000;,
  100;1.000000;1.000000;1.000000;1.000000;,
  101;1.000000;1.000000;1.000000;1.000000;,
  102;1.000000;1.000000;1.000000;1.000000;,
  103;1.000000;1.000000;1.000000;1.000000;,
  104;1.000000;1.000000;1.000000;1.000000;;
 }
}