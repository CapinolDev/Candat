subroutine fortran_main() bind(c, name="fortran_main")
    use iso_c_binding
    implicit none    
    
    integer(c_int) :: received_bytes
    character(kind=c_char), dimension(1000) :: lora_buffer
    integer(c_int) :: key_press
    character(kind=c_char), dimension(2) :: cmd_buffer
    integer :: cmd_idx = 1
    
    interface
        ! Your existing interfaces...
        subroutine pico_print(msg) bind(c, name="pico_print")
            import :: c_char
            character(kind=c_char), dimension(*) :: msg
        end subroutine
        subroutine lora_send_f(msg) bind(c, name="lora_send_f")
            import :: c_char 
            character(kind=c_char), dimension(*) :: msg
        end subroutine
        function lora_receive_f(buffer, max_len) bind(c, name="lora_receive_f")
            import :: c_int, c_char
            integer(c_int) :: lora_receive_f
            character(kind=c_char), dimension(*) :: buffer
            integer(c_int), value :: max_len
        end function lora_receive_f
        
        ! The NEW keyboard function
        function pico_read_char() bind(c, name="pico_read_char")
            import :: c_int
            integer(c_int) :: pico_read_char
        end function pico_read_char
    end interface

    call pico_print("HEWWO :3. Waiting for telemetry or 'GO' command..." // c_null_char)

    do 
        ! --- 1. CHECK FOR SATELLITE TELEMETRY ---
        received_bytes = lora_receive_f(lora_buffer, 1000)
        if (received_bytes > 0) then 
            ! Ensure the string is null-terminated for C printing
            lora_buffer(received_bytes + 1) = c_null_char
            call pico_print("SATELLITE: " // lora_buffer(1:received_bytes+1))
        end if

        ! --- 2. CHECK FOR KEYBOARD COMMANDS (From your laptop) ---
        key_press = pico_read_char()
        if (key_press /= -1) then  ! -1 means no key was pressed
            if (char(key_press) == 'G') then
                cmd_buffer(1) = 'G'
            else if (char(key_press) == 'O' .and. cmd_buffer(1) == 'G') then
                
                call pico_print(" TRANSMITTING GO COMMAND..." // c_null_char)
                call lora_send_f("GO" // c_null_char)
                cmd_buffer(1) = ' ' ! Reset buffer
            end if
        end if

    end do
end subroutine fortran_main