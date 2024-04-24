# trigger
    * Tile 1 and 2: on the front, 6 mm SiPMs
    * Tile 3 and 4: in the showermax layer (9), 3 mm SiPMs

    Trigger is represented by 2 hex digits: 0xNN. 
    The trigger logic is N || N
    The first hex digit represents the 'or' relationship of the 4 trigger tiles:
    And the second hex digit represents the 'and' relationship of the 4 trigger tiles:
	tile 1: 0001
	tile 2: 0010
	tile 3: 0100
	tile 4: 1000

    For example: 0x3C
    The first hex digit 3 is 0011, so it means: tile 1 || tile 2
    The second hex digit C is 1100, which means: tile 3 && tile 4
    So 0x3C means: (tile 1 || tile 2) || (tile 3 && tile 4)
