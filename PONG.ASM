# Note: this source has been modified by David WINTER on 17 SEP 1997
#       (only the syntax changed: it has been converted in CHIPPER)
#
# The source could be optimized to save some bytes, but I didn't wanted
# to modify it since there is no specific interest in this.
#
# NOTE THAT THE ORIGINAL SOURCE HAD SEVERAL ERRORS !!!
#
# ---------------------------------------------------------------------------
#
# From: vervalin@AUSTIN.LOCKHEED.COM (Paul rervalin)
#
#
#        OK folks here it is!  PONG for the HP48SX written in CHIP-48.
#        Some things you should know before you start playing are...
#        1)  This is my first attempt at programming in CHIP-48, so I 
#            know there are probably a few things that could have been
#            done better.
#        2)  The game never ends.  It keeps score, but only up to 9 for
#            each player, then it will roll over to 0.  Sorry, its the
#            only way I could think of to do it.  So, you have to play
#            whoever gets to a number first, wins.
#        3)  It is kind of slow, but then there are two paddles and ball
#            moving around all at once.
#        4)  The player who got the last point gets the serve...
#        5)  Keys 7 and 4 (on the HP48) control the up and down of the 
#            left player and the / and * keys control the up and down of
#            the right player.
#
#        I think that's about it, so have fun!  
# 
# This is a detailed breakdown of the game, sooooooo, if anybody wants to
# make it better, or change it in some way, it might be a little easier.
# Also, about half of the code was in some way extracted from the BRIX
# program.  So, thanks to whoever wrote the original BRIX game.
#
#
# Registers
# ---------
# r0-r3  are scratch registers
# r4     X coord. of score
# r5     Y coord. of score
# r6     X coord. of ball
# r7     Y coord. of ball
# r8     X direction of ball motion
# r9     Y direction of ball motion
# rA     X coord. of left player paddle
# rB     Y coord. of left player paddle
# rC     X coord. of right player paddle
# rD     Y coord. of right player paddle
# rE     Score
# rF     collision detection

Start       CLR
            LOAD    rA,$1       # Set left player X coord.
            LOAD    rB,$C       # Set left player Y coord.
            LOAD    rC,$3F      # Set right player X coord.
            LOAD    rD,$C       # Set right player Y coord.
            LOADI   Paddle      # Get address of paddle sprite

#            DRAW    rA,rB,$6    # Draw left paddle
#            DRAW    rC,rD,$6    # Draw right paddle
            LOAD    rE,$0       # Set score to 00
            CALL    Draw_Score  # Draw score

            LOAD    r6,$1       # Set X coord. of ball to 3
            LOAD    r8,$2       # Set ball X direction to right

Big_Loop    LOAD    r0,$60      # Set r0=delay before ball launch
            LOADD   r0          # Set delay timer to r0

DT_Loop     MOVED   r0          # Read delay timer into r0
            SKE     r0,$0       # Skip next instruction if r0=0
            JUMP    DT_Loop     # Read again delay timer if not 0           
            RAND    r7,$17      # Set Y coord. to rand # AND 23 (0...23)
            ADD     r7,$8       # And adjust it to is 8...31            
            LOAD    r9,$FF      # Set ball Y direction to up
            LOADI   Ball        # Get address of ball sprite
            DRAW    r6,r7,$1    # Draw ball

Padl_Loop   LOADI   Paddle      # Get address of paddle sprite
            DRAW    rA,rB,$6    # Draw left paddle
            DRAW    rC,rD,$6    # Draw right paddle

            LOAD    r0,$4       # Set r0 to KEY $1
            SKUP    r0          # Skip next instruction if KEY in $1 is not pressed
            ADD     rB,$FE      # Subtract 2 from Y coord. of left paddle

            LOAD    r0,$1       # Set r0 to KEY $4
            SKUP    r0          # Skip next instruction if KEY in $4 is not pressed
            ADD     rB,$2       # Add $2 to Y coord. of left paddle

            LOAD    r0,$1F      # Set r0 to max Y coord.  | These three lines are here to
            AND     rB,r0       # AND rB with r0          | adjust the paddle position if
            DRAW    rA,rB,$6    # Draw left paddle        | it is out of the screen

            LOAD    r0,$2       # Set r0 to KEY D
            SKUP    r0          # Skip next instruction if KEY in D is not pressed 
            ADD     rD,$2       # Add $2 to Y coord. of right paddle

            LOAD    r0,$5       # Set r0 to KEY C
            SKUP    r0          # Skip next instruction if KEY in C is not pressed
            ADD     rD,$FE      # Subtract $2 from Y coord. of right paddle

            LOAD    r0,$1F      # Set r0 to max Y coord.  | These three lines are here to
            AND     rD,r0       # AND rD with r0          | adjust the paddle position if
            DRAW    rC,rD,$6    # Draw right paddle       | it is out of the screen

            LOADI   Ball        # Get address of ball sprite
            DRAW    r6,r7,$1    # Draw ball

            ADDR    r6,r8       # Compute next X coord of the ball
            ADDR    r7,r9       # Compute next Y coord of the ball

            LOAD    r0,$3F      # Set r0 to max X location
            AND     r6,r0       # AND r6 with r0

            LOAD    r1,$1F      # Set r1 to max Y location
            AND     r7,r1       # AND r7 with r1

            SKNE    r6,$2       # Skip next instruction if ball not at left
            JUMP    Left_Side   #

            SKNE    r6,$3F      # Skip next instruction if ball not at right
            JUMP    Right_Side  # 

Ball_Loop   SKNE    r7,$1F      # Skip next instruction if ball not at bottom
            LOAD    r9,$FF      # Set Y direction to up

            SKNE    r7,$0       # Skip next instruction if ball not at top
            LOAD    r9,$1       # Set Y direction to down
    
            DRAW    r6,r7,$1    # Draw ball

# END         JUMP    END
            JUMP    Padl_Loop   #

Left_Side   LOAD    r8,$2       # Set X direction to right
            LOAD    r3,$1       # Set r3 to $1 in case left player misses ball
            MOVE    r0,r7       # Set r0 to r7 Y coord. of ball
            SUB     r0,rB       # Subtract position of paddle from ball
            JUMP    Pad_Coll    # Check for collision

Right_Side  LOAD    r8,$FE      # Set X direction to left
            LOAD    r3,$A       # Set r3 to 10 in case right player misses ball
            MOVE    r0,r7       # Set r0 to r7 Y coord. of ball
            SUB     r0,rD       # Subtract position of paddle from ball

Pad_Coll    SKE     rF,$1       # Skip next instruction if ball not above paddle
            JUMP    Ball_Lost   #

            LOAD    r1,$2       # Set r1 to 2
            SUB     r0,r1       # Subtract r1 from r0
            SKE     rF,$1       # Skip next instr. if ball not at top of paddle
            JUMP    Ball_Top    # Ball at top of paddle

            SUB     r0,r1       # Subtract another 2 from r0
            SKE     rF,$1       # Skip next instr. if ball not at middle of paddle
            JUMP    Pad_Hit     # Ball in middle of paddle

            SUB     r0,r1       # Subtract another 2 from r0
            SKE     rF,$1       # Skip next instr. if ball not at bottom of paddle
            JUMP    Ball_Bot    # Ball at bottom of paddle

Ball_Lost   CALL    Draw_Score  # Erase previous score
            ADDR    rE,r3       # Add 1 or 10 to score depending on r3
            CALL    Draw_Score  # Write new score

            LOAD    r6,$3E      # Set ball X coord. to right side
            SKE     r3,$1       # Skip next instr. if right player got point
            LOAD    r6,$2       # Set ball X coord. to left side
            LOAD    r8,$FE      # Set direction to left
            SKE     r3,$1       # Skip next instr. if right player got point
            LOAD    r8,$2       # Set direction to right
            JUMP    Big_Loop    #

Ball_Top    ADD     r9,$FF      # Subtract 1 from r9, ball Y direction
            SKNE    r9,$FE      # Skip next instr. if r9 != FE (-$2)
            LOAD    r9,$FF      # Set r9=FF (-$1)
            JUMP    Pad_Hit 

Ball_Bot    ADD     r9,$1       # Add 1 to r9, ball Y direction
            SKNE    r9,$2       # Skip next instr. if r9 != 02
            LOAD    r9,$1       # Set r9=01

Pad_Hit     ADD     r6,$1       #
            SKNE    r6,$40      #
            ADD     r6,$FE      #
            JUMP    Ball_Loop

Draw_Score  LOADI   Score       # Get address of Score
            BCD     rE          # Stores in memory BCD representation of rE
            READ    r2          # Reads r0...r2 in memory, so the score
            LDSPR   r1          # I points to hex char in r1, so the 1st score char
            LOAD    r4,$14      # Set r4 to the X coord. to draw 1st score char
            LOAD    r5,$0       # Set r5 to the Y coord. to draw 1st score char
            DRAW    r4,r5,$5    # Draw 8*5 sprite at (r4,r5) from M[I], so char r1
            ADD     r4,$15      # Set X to the X coord. of 2nd score char
            LDSPR   r2          # I points to hex char in r2, so 2nd score char
            DRAW    r4,r5,$5    # Draw 8*5 sprite at (r4,r5) from M[I], so char r2
            RTS                 # Return

Paddle      FCB     $80
            FCB     $80
            FCB     $80
            FCB     $80
            FCB     $80
            FCB     $80

Ball        FDB     $80
            FCB     $0

Score       FCB     $0
            FCB     $0
            FCB     $0
            FCB     $0
