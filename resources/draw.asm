# This file can only run properly on the Single-Cycle-MIPS-Simulator, not on MARS. Make sure to launch BlankWindow.exe first.

.data
color_prompt:        .asciiz "Enter color (0-255) or -1 to stop: "
x_prompt:            .asciiz "Enter x of top-left corner (0-255) or -1 to stop: "
y_prompt:            .asciiz "Enter y of top-left corner (0-255) or -1 to stop: "
width_prompt:        .asciiz "Enter width (0-255) or -1 to stop: "
height_prompt:       .asciiz "Enter height (0-255) or -1 to stop: "
draw_rectangle_code: .word   19
draw_bitmap_code:    .word   20
ship_width:          .byte   15
ship_height:         .byte   16
ship: .byte   0,  0,  0,  0,  0,  0, 39, 39, 39,  0,  0,  0,  0,  0,  0
      .byte   0,  0,  0,  0,  0, 39, 39, 39, 39, 39,  0,  0,  0,  0,  0
      .byte   0,  0,  0,  0, 39, 39, 39, 39, 39, 39, 39,  0,  0,  0,  0
      .byte   0,  0,  0,  0, 39, 39, 39, 39, 39, 39, 39,  0,  0,  0,  0
      .byte   0,  0,  0,  0, 39,  0,  0, 39,  0,  0, 39,  0,  0,  0,  0
      .byte   0,  0, 29,  0,  0,  0, 52, 39, 52,  0,  0,  0, 29,  0,  0
      .byte   0,  0, 29,  0,  0,  0, 52, 39, 52,  0,  0,  0, 29,  0,  0
      .byte   0, 29, 29, 29,  0,  0, 52, 39, 52,  0,  0, 29, 29, 29,  0
      .byte   0, 29, 52, 29,  0, 52, 52, 39, 52, 52,  0, 29, 52, 29,  0
      .byte   0, 29, 52, 29, 52, 52, 52, 39, 52, 52, 52, 29, 52, 29,  0
      .byte   0, 29, 52, 52, 52, 52, 52, 39, 52, 52, 52, 52, 52, 29,  0
      .byte   0, 29, 52, 29, 52,  0, 52, 39, 52,  0, 52, 29, 52, 29,  0
      .byte   0, 29, 52, 29,  0,  0, 52,  0, 52,  0,  0, 29, 52, 29,  0
      .byte   0, 29, 52, 29,  0,  0, 52,  0, 52,  0,  0, 29, 52, 29,  0
      .byte   0, 29, 29, 29,  0,  0,  0,  0,  0,  0,  0, 29, 29, 29,  0
      .byte   0,  0, 29,  0,  0,  0,  0,  0,  0,  0,  0,  0, 29,  0,  0             

.text

# defining macro to print a prompt and read an integer from the user
.macro input_val (%prompt)
pr:	la   $a0, %prompt       # load address of prompt for syscall
      li   $v0, 4             # specify Print String service
      syscall
      li   $v0, 5             # specify Read Integer service
      syscall                 # Read the number. After this instruction, the number read is in $v0.
      beq  $v0, -1, exit      # exit the program if user entered -1
      # check if user input is in range 0-255. if not, ask again
      blt  $v0, $zero, pr
      bgt  $v0, 255, pr
.end_macro

            # clearing screen (by drawing a black rectangle on its entire area)
            li   $t0, 0                    # color (index of black in VGA Mode-13H palette)
            li   $t1, 0                    # x
            li   $t2, 0                    # y
            li   $t3, 255                  # width
            li   $t4, 255                  # height
            la   $v0, draw_rectangle_code  # load address of custom syscall code for DRAW_rectangle
            lw   $v0, 0($v0)               # load the actual code to $v0         
            syscall
	
            # moving the ship bitmap from left to right on the screen
            li   $s0, 0                 # x=0 
draw_ship:  la   $t0, ship              # load the array base address (which is the argument we send to the syscall)      
            move $t1, $s0               # store x in $t1
            li   $t2, 220               # store y=220 in $t2
            la   $t3, ship_width        # load address of ship_width variable
            lb   $t3, 0($t3)            # load the value of ship_width to $t3
            la   $t4, ship_height       # load address of ship_height variable
            lb   $t4, 0($t4)            # load the value of ship_height to $t4
            la   $v0, draw_bitmap_code  # load address of custom syscall code for DRAW_bitmap
            lw   $v0, 0($v0)            # load the actual code to $v0         
            syscall
            li   $a0, 5                 # sleep for 5 milliseconds
            li   $v0, 32                # specify sleep service (exclusive to MARS, though not important since this code will run on our emulator, not MARS)
            syscall
            addi $s0, $s0, 1            # increment x
            li   $s1, 255
            subu $s1, $s1, $t3          # 255 - ship_width
            blt  $s0, $s1, draw_ship    # repeat as long as x < 255 - ship_width
	
            # getting color and storing in $t0
loop:       input_val (color_prompt)
            move $t0, $v0
      
            # getting x and storing in $t1
            input_val (x_prompt)
            move $t1, $v0
      
            # getting y and storing in $t2
            input_val (y_prompt)
            move $t2, $v0
      
            # getting width and storing in $t3
            input_val (width_prompt)
            move $t3, $v0
      
            # getting height and storing in $t4
            input_val (height_prompt)
            move $t4, $v0
      
            la   $v0, draw_rectangle_code  # load address of custom syscall code for DRAW_rectangle
            lw   $v0, 0($v0)               # load the actual code to $v0
            syscall
            j loop                         # drawing rectangles repeatedly until user enters -1
      
            # The program is finished. Exit.
exit:       li   $v0, 10          # system call for exit
            syscall               # Exit! 
      