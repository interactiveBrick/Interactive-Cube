# Cube sequencer


## Top - Step editor

      back
    0 1 2 3
    4 5 6 7
    8 9 A B
    C D E F
     front

    0-F - Toggle step for current instrument and track (solid = step enabled)

## Left - CC Value 1 for selected step

            top
          F E D C
    back  B A 9 8  front
          7 6 5 4
          3 2 1 0

## Right - CC Value 2 for selected step

             top
           C D E F
    front  8 9 A B  back
           4 5 6 7
           0 1 2 3

## Front - Select editing track + sample

      top
    0 1 2 3
    4 5 6 7
    8 9 A B
    C D E F
     bottom

    0-3 Select instrument msb (solid = used, blinking = current)
    4-7 Select instrument lsb (solid = used, blinking = current)

    C-F Select track to edit (solid = selected)

## Bottom - Live trigger

     front
    0 1 2 3
    4 5 6 7
    8 9 A B
    C D E F
     back

    0-F - Trigger sample (solid = playing)


## Back - Select playing tracks + sample

      top
    0 1 2 3
    4 5 6 7
    8 9 A B
    C D E F
     bottom

    0-3 Select track to play (solid = playing, blinking = cued, click = cue, could use all four rows for highlighting)
