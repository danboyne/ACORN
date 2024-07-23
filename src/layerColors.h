//
// Define colors to be used for each routing and via layer, starting with layer #0:
//
//                   Red  Green  Blue  Opacity
//                  ----- -----  ----- -------
const int RGBA[] = { 0xFF, 0x00,  0x00,  0x80,   // Routing layer  0 = semi-transparent red
                     0xCC, 0x00,  0x00,  0xFF,   //     Via layer    = opaque dark red

                     0x00, 0x00,  0xFF,  0x80,   // Routing layer  1 = semi-transparent blue
                     0x00, 0x00,  0xCC,  0xFF,   //     Via layer    = opaque dark blue

                     0x00, 0xFF,  0x00,  0x80,   // Routing layer  2 = semi-transparent green
                     0x00, 0xCC,  0x00,  0xFF,   //     Via layer    = opaque dark green

                     0x99, 0x66,  0x33,  0x80,   // Routing layer  3 = semi-transparent brown
                     0x73, 0x4D,  0x26,  0xFF,   //     Via layer    = opaque dark brown

                     0xFF, 0x00,  0xFF,  0x80,   // Routing layer  4 = semi-transparent pink
                     0xCC, 0x00,  0xCC,  0xFF,   //     Via layer    = opaque dark pink

                     0x99, 0x00,  0x99,  0x80,   // Routing layer  5 = semi-transparent purple
                     0x66, 0x00,  0x66,  0xFF,   //     Via layer    = opaque dark purple

                     0x80, 0x80,  0x00,  0x80,   // Routing layer  6 = semi-transparent olive
                     0xE6, 0xE6,  0x00,  0xFF,   //     Via layer    = opaque dark yellow

                     0x66, 0xCC,  0xFF,  0x80,   // Routing layer  7 = semi-transparent light blue
					 0x66, 0xCC,  0xFF,  0xFF,   //     Via layer    = opaque light blue

                     0xF3, 0xA8,  0x31,  0x80,   // Routing layer  8 = semi-transparent burnt orange
					 0xF3, 0xA8,  0x31,  0xFF,   //     Via layer    = opaque burnt orange

					 0xFF, 0xFF,  0x00,  0x80    // Routing layer  9 = semi-transparent yellow
                   };
