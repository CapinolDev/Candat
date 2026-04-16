subroutine fortran_main() bind(c, name="fortran_main")
    use iso_c_binding
    implicit none    
    integer :: iX
    integer(c_int) :: recieved
    character(kind=c_char), dimension(1000) :: inBuffer
    interface
        subroutine fortran_is_here() bind(c, name="fortran_is_here")
        end subroutine fortran_is_here

        subroutine pico_sleep(ms) bind(c,name="pico_sleep")
        import :: c_int
        integer(c_int), value :: ms
        end subroutine
        subroutine pico_print(msg) bind(c, name="pico_print")
            import :: c_char
            character(kind=c_char), dimension(*) :: msg
        end subroutine pico_print

        subroutine lora_send_f(msg) bind(c,name='lora_send_f')
            import :: c_char 
            character(kind=c_char), dimension(*) :: msg
        end subroutine lora_send_f

        function lora_receive_f(buffer, max_len) bind(c, name="lora_receive_f")
            import :: c_int, c_char
            implicit none
            integer(c_int) :: lora_receive_f
            character(kind=c_char), dimension(*) :: buffer
            integer(c_int), value :: max_len
        end function lora_receive_f
        subroutine testPrint() bind(c,name='testPrint')
        end subroutine testPrint
    end interface
    call testPrint()
    call pico_print("LoRa started. Hi :)"//c_null_char)
    
    do 
    recieved = lora_receive_f(inBuffer,1000)
    if (recieved /= 0) then 
        if ((inBuffer(1) == "G") .and. (inBuffer(2) == "O")) then 
            call pico_print("Prikaz prijat"//c_null_char)    
        else if ((inBuffer(1) == "P") .and. (inBuffer(2) == "I") .and. (inBuffer(3) == "N") .and. (inBuffer(4) == "G")) then
            call pico_print("Ping?"//c_null_char)    
            call lora_send_f("PONG"//c_null_char)        
            call pico_print("Pong"//c_null_char)    
        
        else 
            call pico_print("Unknown comm recieved"//c_null_char)    
        end if
    end if

    call pico_sleep(300)
    end do

   
end subroutine fortran_main