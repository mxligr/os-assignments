#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <unistd.h>

int file_size;      // global variable that holds the size of the file that we need to map in memory at requirement 2.6 (I use this variable in main for unmapping the file)
char *shm_data = NULL;      // global variable that points to shared memory
char *data = NULL;          // global variable that points to memory mapping
int read_fd;                // file descriptor for reading pipe ("REQ_PIPE_12170")
int write_fd;               // file descriptor for writing pipe ("RESP_PIPE_12170")

// method that reads a message(string-field) sent by the tester and returns a string with it
char* get_pipe_message() {

    // read the first byte of the message, which contains the number of bytes of the message
    char bytes_read = 0;
    read(read_fd, &bytes_read, sizeof(char));

    // allocate space for a string that will contain the message read from the pipe
    char *message_received = (char*)malloc(sizeof(char)*((int)bytes_read + 1));

    // read the message from the pipe
    for(int i = 0; i < (int)bytes_read; i++) {
        char c;
        read(read_fd, &c, sizeof(char));
        message_received[i] = c;
    }

    // append end of string character
    message_received[(int)bytes_read] = '\0';
    
    // return message
    return message_received;
}

// method that gets a string and writes it in the form of a string-field into the pipe
void send_string_message(char *message_sent) {

    // the first byte of a string-field represents the size of the string-field, so get the length of the message and write it in the pipe
    char size_of_message = strlen(message_sent);
    write(write_fd, &size_of_message, sizeof(char));

    // traverse the string character by character, and write the characters in the pipe
    for (int i = 0; i < size_of_message ; i++) {
        write(write_fd, &message_sent[i], sizeof(char));
    }
}

// method that gets an unsigned int and writes it in the form of a number-field into the pipe
void send_number_message(unsigned int num) {

    // write the bytes of the unsigned int number into the pipe
    write(write_fd, &num, sizeof(unsigned int));
}

// 2.3 - "PING"
void ping_request() {
    // In response to a ping request, your program must simply send back the following response : "PING" "PONG" 12170

    // send "PING"
    send_string_message("PING");
    // send "PONG"
    send_string_message("PONG");
    // send 12170
    send_number_message(12170);
}

// 2.4 - "CREATE_SHM" 5075507
char* create_shm_request() {

    // read number 5075507 from pipe
    unsigned int num_bytes;
    read(read_fd, &num_bytes, sizeof(num_bytes));

    // declare name of shared memory
    char *shm_name = "/Zee4mNhp";

    // create shared memory
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0664);

    if (shm_fd == -1) {
        // creating failed - send error message
        send_string_message("CREATE_SHM");
        send_string_message("ERROR");
    } else {
        // set the length of the shared memory region to 5075507 bytes
        off_t length = (off_t)num_bytes;
        int res = ftruncate(shm_fd, length);
        if (res == -1) {
            // setting length failed - send error message
            send_string_message("CREATE_SHM");
            send_string_message("ERROR");
        } else {
            // map the shared memory 
            shm_data = (char*)mmap(NULL, num_bytes, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
            if (shm_data == MAP_FAILED) {
                // mapping failed - send error message
                send_string_message("CREATE_SHM");
                send_string_message("ERROR");
            } else {
                send_string_message("CREATE_SHM");
                send_string_message("SUCCESS");
            }
        }
    }

    close(shm_fd);
    return shm_data;
}

// 2.5 - "WRITE_TO_SHM" <offset> <value>
void write_to_shm_request() {

    // read offset from pipe
    unsigned int offset;
    read(read_fd, &offset, sizeof(unsigned int));

    // read value from pipe
    unsigned int value;
    read(read_fd, &value, sizeof(unsigned int));

    // validate if the given offset is inside the shared memory region
    if (offset > 5075507) {
        send_string_message("WRITE_TO_SHM");
        send_string_message("ERROR");
    } else {
        // validate if all the bytes that should be written also correspond to offsets inside the shared memory region.
        if (offset + sizeof(value) > 5075507) {
            send_string_message("WRITE_TO_SHM");
            send_string_message("ERROR");
        } else {
            unsigned char b1 = (value >> 24);   // first byte of the unsigned int 
            unsigned char b2 = (value >> 16);   // second byte of the unsigned int
            unsigned char b3 = (value >> 8);    // third byte of the unsigned int
            unsigned char b4 = value;           // last byte of the unsigned int
            // printf("%x\n%x\n%x\n%x\n", (unsigned char)b1, (unsigned char)b2, (unsigned char)b3, (unsigned char)b4);
            *(shm_data + offset + 3) = b1;
            *(shm_data + offset + 2) = b2;
            *(shm_data + offset + 1) = b3;
            *(shm_data + offset) = b4;
            send_string_message("WRITE_TO_SHM");
            send_string_message("SUCCESS");
        }
    }
}

// 2.6 - "MAP_FILE" <file_name>
void map_file_request() {

    // read file name from pipe
    char *file_name = get_pipe_message();

    // open file
    int file_fd = open(file_name, O_RDONLY);

    // get size of file
    file_size = lseek(file_fd, 0, SEEK_END);

    // move cursor back to the start of the file
    lseek(file_fd, 0, SEEK_SET);

    // map the file
    data = (char*)mmap(NULL, file_size, PROT_READ, MAP_SHARED, file_fd, 0);
    if (data == MAP_FAILED) {
        send_string_message("MAP_FILE");
        send_string_message("ERROR");
    } else {
        send_string_message("MAP_FILE");
        send_string_message("SUCCESS");
    }

    // after mapping, close file descriptor
    close(file_fd);
}

// 2.7 - "READ_FROM_FILE_OFFSET" <offset> <no_of_bytes>
void read_from_file_offset_request() {

    // read offset from pipe
    unsigned int offset;
    read(read_fd, &offset, sizeof(unsigned int));

    // read number of bytes from pipe
    unsigned int no_of_bytes;
    read(read_fd, &no_of_bytes, sizeof(unsigned int));

    // check if the required offset added with the number of bytes to be read gives a number smaller than the file size.
    if (offset + no_of_bytes > file_size) {
        send_string_message("READ_FROM_FILE_OFFSET");
        send_string_message("ERROR");
    } else {
        for(int i = offset; i < offset + no_of_bytes; i++) {
            int j = i - offset;
            *(shm_data + j) = *(data + i);
        }
        send_string_message("READ_FROM_FILE_OFFSET");
        send_string_message("SUCCESS");
    }
}

// output: b1 b2 b3 b4
// helper function that combines 4 1 byte chars into one 4 byte unsigned int
unsigned int combine(unsigned char b1, unsigned char b2, unsigned char b3, unsigned char b4) {
    unsigned int x = 0;
    x |= (unsigned int)b1 << 24;
    x |= (unsigned int)b2 << 16;
    x |= (unsigned int)b3 << 8;
    x |= (unsigned int)b4;
    return x;
}

// 2.8 - "READ_FROM_FILE_SECTION" <section_no> <offset> <no_of_bytes>
void read_from_file_section_request() {

    // read section number from pipe
    unsigned int section_no;
    read(read_fd, &section_no, sizeof(unsigned int));

    // read offset from pipe
    unsigned int offset;
    read(read_fd, &offset, sizeof(unsigned int));

    // read number of bytes from pipe
    unsigned int no_of_bytes;
    read(read_fd, &no_of_bytes, sizeof(unsigned int));
    // printf("section_no: %u\noffset: %u\nno_of_bytes: %u\n", section_no, offset, no_of_bytes);

    // check the number of sections
    unsigned char no_of_sections = *(data + 4);
    if((unsigned int)no_of_sections < section_no) {
        send_string_message("READ_FROM_FILE_SECTION");
        send_string_message("ERROR");
    } else {
        // jump to the header of the section we want and read the section's offset
        unsigned char b1, b2, b3, b4;
        b1 = *(data + 5 + ((section_no - 1) * 18) + 10);
        b2 = *(data + 5 + ((section_no - 1) * 18) + 10 + 1);
        b3 = *(data + 5 + ((section_no - 1) * 18) + 10 + 2);
        b4 = *(data + 5 + ((section_no - 1) * 18) + 10 + 3);
        unsigned int section_offset = combine(b4, b3, b2, b1);
        // read the section's size
        b1 = *(data + 5 + ((section_no - 1) * 18) + 10 + 4);
        b2 = *(data + 5 + ((section_no - 1) * 18) + 10 + 5);
        b3 = *(data + 5 + ((section_no - 1) * 18) + 10 + 6);
        b4 = *(data + 5 + ((section_no - 1) * 18) + 10 + 7);
        unsigned int section_size = combine(b4, b3, b2, b1);
        if (offset + no_of_bytes > section_size) {
            send_string_message("READ_FROM_FILE_SECTION");
            send_string_message("ERROR");
        } else {
            unsigned int start_from_here = section_offset + offset;
            if(start_from_here > file_size) {
                send_string_message("READ_FROM_FILE_SECTION");
                send_string_message("ERROR");
            } else {
                for (unsigned int i = start_from_here; i < (start_from_here + no_of_bytes); i++) {
                    unsigned int j = i - start_from_here;
                    *(shm_data + j) = *(data + i);
                }
                send_string_message("READ_FROM_FILE_SECTION");
                send_string_message("SUCCESS");
            }
        }
    }
}

// 2.9 - "READ_FROM_LOGICAL_SPACE_OFFSET" <logical_offset> <no_of_bytes>
// NOT IMPLEMENTED
void read_from_logical_space_offset_request() {

    // read logical offset from pipe
    unsigned int logical_offset;
    read(read_fd, &logical_offset, sizeof(unsigned int));

    // read number of bytes from pipe
    unsigned int no_of_bytes;
    read(read_fd, &no_of_bytes, sizeof(unsigned int));

    send_string_message("READ_FROM_LOGICAL_SPACE_OFFSET");
    send_string_message("ERROR");
}

// function that handles the requests given by the tester
void handle_request(char *message_received){

    // 2.3 Ping Request
    if(strcmp(message_received, "PING") == 0) {
        ping_request();
    }
    
    // 2.4 Request to Create a Shared Memory Region
    if(strcmp(message_received, "CREATE_SHM") == 0) {
        create_shm_request();
    }

    // 2.5 Write to Shared Memory Request  
    if(strcmp(message_received, "WRITE_TO_SHM") == 0) {
        write_to_shm_request();
    }    

     // 2.6 Memory-Map File Request
    if(strcmp(message_received, "MAP_FILE") == 0) {
        map_file_request();
    }
    
    // 2.7 Read From File Offset Request
    if(strcmp(message_received, "READ_FROM_FILE_OFFSET") == 0) {
        read_from_file_offset_request();
    
    }
    
    // 2.8 Read From File Section Request
    if(strcmp(message_received, "READ_FROM_FILE_SECTION") == 0) {
        read_from_file_section_request();
    } 

    // 2.9 Read From Logical Memory Space Offset Request
    if(strcmp(message_received, "READ_FROM_LOGICAL_SPACE_OFFSET") == 0) {
        read_from_logical_space_offset_request();
    }
}

int main() {

    // 2.2 Pipe-Based Connection
    // 1. creates a pipe named “RESP PIPE 12170”;
    int result = mkfifo("RESP_PIPE_12170", 0600);
    if (result < 0) {
        printf("ERROR\ncannot create the response pipe | cannot open the request pipe\n");
        return -1;
    }

    // 2. opens for reading a pipe named “REQ PIPE 12170”, supposed to be created automatically
    // by our testing program;
    read_fd = open("REQ_PIPE_12170", O_RDONLY);
    if(read_fd < 0) {
        printf("ERROR\ncannot create the response pipe | cannot open the request pipe\n");
        return -1;
    }

    // 3. opens for writing the pipe “RESP PIPE 12170” that was previously created at step 1;
    write_fd = open("RESP_PIPE_12170", O_WRONLY);
    if(write_fd < 0) {
        printf("ERROR\ncannot create the response pipe | cannot open the request pipe\n");
        return -1;
    }

    // 4. writes the following request message on the “RESP PIPE 12170” pipe : "CONNECT"
    send_string_message("CONNECT");

    printf("SUCCESS\n");
    // now we get the message sent by the tester and we handle the request
    while(1) {
        char *message_received = get_pipe_message();
        // 2.10 Exit Request
        // When getting this type of message, your program must close the connection / request pipe, close and remove the response pipe and terminate.
        if(strcmp(message_received, "EXIT") == 0) {
            close(read_fd);
            close(write_fd);
            unlink("RESP_PIPE_12170");
            shm_unlink("/Zee4mNhp");
            munmap(shm_data, 5075507);
            shm_data = NULL;
            munmap(data, file_size);
            data = NULL;
            break;
        } else {
            handle_request(message_received);
        }
    }

    return 0;

}
