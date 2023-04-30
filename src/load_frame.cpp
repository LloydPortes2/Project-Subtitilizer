bool load_frame(const char* filename, int* width, int* height, unsigned char ** data){

  *width = 100;
  *height = 100;
  *data = new unsigned char [100 * 100 * 3];

  auto ptr = *data;
  for (int x = 0; x < 100; x++) {
    for (int y = 0; y < 100; y++){
      *ptr++ = 0xff;
      *ptr++ = 0x00;
      *ptr++ = 0x00;
    }
  }
  return false;
}
