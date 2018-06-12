7950 // PC keyboard interface constants
7951 
7952 #define KBSTATP         0x64    // kbd controller status port(I)
7953 #define KBS_DIB         0x01    // kbd data in buffer
7954 #define KBDATAP         0x60    // kbd data port(I)
7955 
7956 #define NO              0
7957 
7958 #define SHIFT           (1<<0)
7959 #define CTL             (1<<1)
7960 #define ALT             (1<<2)
7961 
7962 #define CAPSLOCK        (1<<3)
7963 #define NUMLOCK         (1<<4)
7964 #define SCROLLLOCK      (1<<5)
7965 
7966 #define E0ESC           (1<<6)
7967 
7968 // Special keycodes
7969 #define KEY_HOME        0xE0
7970 #define KEY_END         0xE1
7971 #define KEY_UP          0xE2
7972 #define KEY_DN          0xE3
7973 #define KEY_LF          0xE4
7974 #define KEY_RT          0xE5
7975 #define KEY_PGUP        0xE6
7976 #define KEY_PGDN        0xE7
7977 #define KEY_INS         0xE8
7978 #define KEY_DEL         0xE9
7979 
7980 // C('A') == Control-A
7981 #define C(x) (x - '@')
7982 
7983 static uchar shiftcode[256] =
7984 {
7985   [0x1D] CTL,
7986   [0x2A] SHIFT,
7987   [0x36] SHIFT,
7988   [0x38] ALT,
7989   [0x9D] CTL,
7990   [0xB8] ALT
7991 };
7992 
7993 static uchar togglecode[256] =
7994 {
7995   [0x3A] CAPSLOCK,
7996   [0x45] NUMLOCK,
7997   [0x46] SCROLLLOCK
7998 };
7999 
8000 static uchar normalmap[256] =
8001 {
8002   NO,   0x1B, '1',  '2',  '3',  '4',  '5',  '6',  // 0x00
8003   '7',  '8',  '9',  '0',  '-',  '=',  '\b', '\t',
8004   'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  // 0x10
8005   'o',  'p',  '[',  ']',  '\n', NO,   'a',  's',
8006   'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',  // 0x20
8007   '\'', '`',  NO,   '\\', 'z',  'x',  'c',  'v',
8008   'b',  'n',  'm',  ',',  '.',  '/',  NO,   '*',  // 0x30
8009   NO,   ' ',  NO,   NO,   NO,   NO,   NO,   NO,
8010   NO,   NO,   NO,   NO,   NO,   NO,   NO,   '7',  // 0x40
8011   '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
8012   '2',  '3',  '0',  '.',  NO,   NO,   NO,   NO,   // 0x50
8013   [0x9C] '\n',      // KP_Enter
8014   [0xB5] '/',       // KP_Div
8015   [0xC8] KEY_UP,    [0xD0] KEY_DN,
8016   [0xC9] KEY_PGUP,  [0xD1] KEY_PGDN,
8017   [0xCB] KEY_LF,    [0xCD] KEY_RT,
8018   [0x97] KEY_HOME,  [0xCF] KEY_END,
8019   [0xD2] KEY_INS,   [0xD3] KEY_DEL
8020 };
8021 
8022 static uchar shiftmap[256] =
8023 {
8024   NO,   033,  '!',  '@',  '#',  '$',  '%',  '^',  // 0x00
8025   '&',  '*',  '(',  ')',  '_',  '+',  '\b', '\t',
8026   'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  // 0x10
8027   'O',  'P',  '{',  '}',  '\n', NO,   'A',  'S',
8028   'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',  // 0x20
8029   '"',  '~',  NO,   '|',  'Z',  'X',  'C',  'V',
8030   'B',  'N',  'M',  '<',  '>',  '?',  NO,   '*',  // 0x30
8031   NO,   ' ',  NO,   NO,   NO,   NO,   NO,   NO,
8032   NO,   NO,   NO,   NO,   NO,   NO,   NO,   '7',  // 0x40
8033   '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
8034   '2',  '3',  '0',  '.',  NO,   NO,   NO,   NO,   // 0x50
8035   [0x9C] '\n',      // KP_Enter
8036   [0xB5] '/',       // KP_Div
8037   [0xC8] KEY_UP,    [0xD0] KEY_DN,
8038   [0xC9] KEY_PGUP,  [0xD1] KEY_PGDN,
8039   [0xCB] KEY_LF,    [0xCD] KEY_RT,
8040   [0x97] KEY_HOME,  [0xCF] KEY_END,
8041   [0xD2] KEY_INS,   [0xD3] KEY_DEL
8042 };
8043 
8044 
8045 
8046 
8047 
8048 
8049 
8050 static uchar ctlmap[256] =
8051 {
8052   NO,      NO,      NO,      NO,      NO,      NO,      NO,      NO,
8053   NO,      NO,      NO,      NO,      NO,      NO,      NO,      NO,
8054   C('Q'),  C('W'),  C('E'),  C('R'),  C('T'),  C('Y'),  C('U'),  C('I'),
8055   C('O'),  C('P'),  NO,      NO,      '\r',    NO,      C('A'),  C('S'),
8056   C('D'),  C('F'),  C('G'),  C('H'),  C('J'),  C('K'),  C('L'),  NO,
8057   NO,      NO,      NO,      C('\\'), C('Z'),  C('X'),  C('C'),  C('V'),
8058   C('B'),  C('N'),  C('M'),  NO,      NO,      C('/'),  NO,      NO,
8059   [0x9C] '\r',      // KP_Enter
8060   [0xB5] C('/'),    // KP_Div
8061   [0xC8] KEY_UP,    [0xD0] KEY_DN,
8062   [0xC9] KEY_PGUP,  [0xD1] KEY_PGDN,
8063   [0xCB] KEY_LF,    [0xCD] KEY_RT,
8064   [0x97] KEY_HOME,  [0xCF] KEY_END,
8065   [0xD2] KEY_INS,   [0xD3] KEY_DEL
8066 };
8067 
8068 
8069 
8070 
8071 
8072 
8073 
8074 
8075 
8076 
8077 
8078 
8079 
8080 
8081 
8082 
8083 
8084 
8085 
8086 
8087 
8088 
8089 
8090 
8091 
8092 
8093 
8094 
8095 
8096 
8097 
8098 
8099 
