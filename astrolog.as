@0309  ; Astrolog (6.10) default settings file astrolog.as

-z 8:00W                ; Default time zone     [hours W or E of GMT   ]
-z0 Autodetect          ; Default Daylight time [0 standard, 1 daylight]
-zl 122W19:59 47N36:35  ; Default location      [longitude and latitude]
-zv 161ft               ; Default elevation     [in feet or meters     ]
-zj "Current moment now" "Seattle, WA" ; Default name and location

-Yz 0   ; Time minute addition to be used when "now" charts are off.
;-n      ; Comment out this line to not start with chart for "now".

_s      ; Zodiac selection          ["_s" is tropical, "=s" is sidereal]
:s 0    ; Zodiac offset value       [Change "0" to desired ayanamsa    ]
-A 5    ; Number of aspects         [Change "5" to desired number      ]
-c Plac ; House system              [Change "Plac" to desired system   ]
_k      ; Ansi color text           ["=k" is color, "_k" is normal     ]
:d 48   ; Searching divisions       [Change "48" to desired divisions  ]
_b0     ; Print zodiac seconds      ["_b0" to minute, "=b0" to second  ]
=b      ; Use ephemeris files       ["=b" uses them, "_b" doesn't      ]
=C      ; Show house cusp objects   ["_C" hides them, "=C" shows them  ]
_v7     ; Show esoteric rulerships  ["=v7" shows them, "_v7" hides them]
:w 4    ; Wheel chart text rows     [Change "4" to desired wheel rows  ]
:I 80   ; Text screen columns       [Change "80" to desired columns    ]
-YQ 24  ; Text screen scroll limit  [Change "24" or set to "0" for none]
_Yd     ; European date format      ["_Yd" is MDY, "=Yd" is DMY        ]
_Yt     ; European time format      ["_Yt" is AM/PM, "=Yt" is 24 hour  ]
_Yv     ; European length units     ["_Yv" is imperial, "=Yv" is metric]
_Yr     ; Show rounded positions    ["=Yr" rounds, "_Yr" doesn't       ]
=YC     ; Smart cusp displays       ["=YC" is smart, "_YC" is normal   ]
=Y8     ; Clip text to end of line  ["=Y8" clips, "_Y8" doesn't clip   ]


; DEFAULT RESTRICTIONS:

-YR 0 10     1 0 0 0 0 0 0 0 0 0 0    ; Planets
-YR 11 21    0 0 0 0 0 0 1 0 0 1 1    ; Minor planets
-YR 22 33    0 1 1 1 1 1 1 1 1 0 1 1  ; House cusps
-YR 34 42    1 1 1 1 1 1 1 1 1        ; Uranians

; DEFAULT TRANSIT RESTRICTIONS:

-YRT 0 10    1 0 1 0 0 0 0 0 0 0 0    ; Planets
-YRT 11 21   0 0 0 0 0 0 1 0 1 1 1    ; Minor planets
-YRT 22 33   1 1 1 1 1 1 1 1 1 1 1 1  ; House cusps
-YRT 34 42   0 0 0 0 0 0 0 0 0        ; Uranians

-YR0 0 0  ; Restrict sign, direction changes


; DEFAULT ASPECT ORBS:

-YAo 1 5     7 7 7 7 6      ; Major aspects
-YAo 6 11    3 3 3 3 2 2    ; Minor aspects
-YAo 12 18   1 1 1 1 1 1 1  ; Obscure aspects

; DEFAULT MAX PLANET ASPECT ORBS:

-YAm 0 10    360 360 360 360 360 360 360 360 360 360 360      ; Planets
-YAm 11 21   360 360 360 360 360   2   2   2 360 360   2      ; Minor planets
-YAm 22 33   360 360 360 360 360 360 360 360 360 360 360 360  ; Cusp objects
-YAm 34 42   360 360 360 360 360 360 360 360 360              ; Uranians
-YAm 43 43     2                                              ; Fixed stars

; DEFAULT PLANET ASPECT ORB ADDITIONS:

-YAd 0 10    1 1 1 0 0 0 0 0 0 0 0    ; Planets
-YAd 11 21   0 0 0 0 0 0 0 0 0 0 0    ; Minor planets
-YAd 22 33   0 0 0 0 0 0 0 0 0 0 0 0  ; Cusp objects
-YAd 34 42   0 0 0 0 0 0 0 0 0        ; Uranians
-YAd 43 43   0                        ; Fixed stars


; DEFAULT INFLUENCES:

-Yj 0 10    30 30 25 10 10 10 10 10 10 10 10     ; Planets
-Yj 11 21   5 5 5 5 5 5 5 5 5 5 5                ; Minor planets
-Yj 22 33   20 10 10 10 10 10 10 10 10 15 10 10  ; Cusp objects
-Yj 34 42   5 3 3 3 3 3 3 3 3                    ; Uranians
-Yj 43 43   2                                    ; Fixed stars

-YjC 1 12   20 0 0 10 0 0 5 0 0 15 0 0  ; Houses

-YjA 1 5    1.0 0.8 0.8 0.6 0.6          ; Major aspects
-YjA 6 11   0.4 0.4 0.2 0.2 0.2 0.2      ; Minor aspects
-YjA 12 18  0.1 0.1 0.1 0.1 0.1 0.1 0.1  ; Obscure aspects

; DEFAULT TRANSIT INFLUENCES:

-YjT 0 10   10 10 4 8 9 20 30 35 40 45 50  ; Planets
-YjT 11 21  30 15 15 15 15 30 1 1 1 1 1    ; Minor planets
-YjT 34 42  2 50 50 50 50 50 50 50 50      ; Uranians
-YjT 43 43  80                             ; Fixed stars

-Yj0 20 10 15 5  ; In ruling sign, exalted sign, ruling house, exalted house
-Yj7 10 10 10 5 5 5  ; Esoteric, Hierarchical, Ray ruling - sign, house


; DEFAULT RAYS:

-Y7C 1 12   17 4 2 37 15 26 3 4 456 137 5 26  ; Signs
-Y7O 0 10   3 2 4 4 5 6 2 3 7 6 1             ; Planets
-Y7O 34 42  1 0 0 0 0 0 0 0 0                 ; Uranians


; DEFAULT COLORS:

-YkO 0 10   Yel Ele Ele Ele Ele Ele Ele Ele Ele Ele Ele      ; Planet colors
-YkO 11 21  Mag Mag Mag Mag Mag DkC DkC DkC DkC DkC DkC      ; Minor colors
-YkO 22 33  Ele Ele Ele Ele Ele Ele Ele Ele Ele Ele Ele Ele  ; Cusp colors
-YkO 34 42  Pur Pur Pur Pur Pur Pur Pur Pur Pur              ; Uranian colors

-YkA 1 5    Yel Blu Red Gre Cya          ; Major aspect colors
-YkA 6 11   Mag Mag Ora Ora DkC DkC      ; Minor aspect colors
-YkA 12 18  DkC Mar Pur Pur Mar Mar Pur  ; Obscure aspect colors

-YkC        Red Yel Gre Blu                      ; Element colors
-Yk7 1 7    Red Blu Gre Yel Ora Mag Pur          ; Ray colors
-Yk0 1 7    Red Ora Yel Gre Cya Blu Pur          ; Rainbow colors
-Yk  0 8    Bla Whi LtG Gra Mar DkG DkC DkB Mag  ; Main colors


; GRAPHICS DEFAULTS:

_X              ; Graphics chart flag ["_X" is text, "=X" is graphics]
:Xw 480 480     ; Default X and Y resolution
:Xs 200         ; Character scale [100-400]
:XS 100         ; Graphics text scale [100-400]
:Xbb            ; Bitmap file type
:YXG 1111       ; Glyph selections [Capricorn, Uranus, Pluto, Lilith]
:YXg 20         ; Aspect grid cells
:YX7 600        ; Esoteric ray column influence width
:YXf 2          ; Use actual fonts
:YXp 0          ; PostScript paper orientation
:YXp0 8.5 11.0  ; PostScript paper X and Y inch sizes

; astrolog.as
