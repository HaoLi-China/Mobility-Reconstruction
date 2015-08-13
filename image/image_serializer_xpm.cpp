
#include "image_serializer_xpm.h"
#include "image.h"
#include "image_store.h"
#include "../basic/basic_types.h"
#include "../basic/logger.h"

#include <sstream>
#include <string.h>


    inline int htoi(char digit) {
        if(digit >= '0' && digit <= '9') {
            return digit - '0' ;
        }
        if(digit >= 'a' && digit <= 'f') {
            return digit - 'a' + 10 ;
        }
        if(digit >= 'A' && digit <= 'F') {
            return digit - 'A' + 10 ;
        }   
        Logger::err("Image") 
            << "XPM Image reader: hex digit to integer: invalid digit: \'" 
            << digit << "\'" << std::endl ;
        abort() ;
        return 0 ; // to avoid a compiler warning.
    }


    Image* ImageSerializer_xpm::serialize_read(std::istream& stream) {

        // Well, ugly code ahead, 
        //   sorry about that, but I really needed 
        //   an XPM image reader ...

        int num_colors ;
        int chars_per_pixels ;
        int width ;
        int height ;

        // _______________________ header
        {
            char* header = next_xpm_data(stream) ;
            if(header == nil) {
                Logger::err("Image") 
                    << "XPM image input: unexpected end of file" << std::endl ;
                return nil ;
            }
            std::istringstream in(header) ;
            in >> width >> height >> num_colors >> chars_per_pixels ;
            if(num_colors > 256) {
                Logger::err("Image") 
                    << "XPM image input: too many colors ("
                    << num_colors
                    << ")" << std::endl ;
                Logger::err("Image") 
                    << "  should not be greater than 256" << std::endl ;
                return nil ;
            }

            switch(chars_per_pixels) {
            case 1:
                return read_xpm_1_byte_per_pixel(
                    width, height, num_colors, stream
                ) ;
                break ;
            case 2:
                return read_xpm_2_bytes_per_pixel(
                    width, height, num_colors, stream
                ) ;
                break ;
            default:
                Logger::err("Image") 
                    << "XPM image input: invalid chars per pixels ("
                    << chars_per_pixels << ")" << std::endl ;
                Logger::err("Image") << "  should be 2" << std::endl ;
                return nil ;
                break ;
            }
        }
    }
    
    Image* ImageSerializer_xpm::read_xpm_2_bytes_per_pixel(        
        int width, int height, int num_colors, std::istream& stream
    ) {

        // Converts a two-digit XPM color code into
        //  a color index.
        static int conv_table[256][256] ;

        // For checking, put a negative value to
        //  detect invalid color codes.
        for(int k1=0; k1 < 256; k1++) {
            for(int k2=0; k2 < 256; k2++) {
                conv_table[k1][k2] = -1 ;
            }
        }
    
        // _______________________  colormap
    
        typedef Numeric::uint8 byte ;

        Colormap* colormap = new Colormap(num_colors) ;

        for(int entry_num=0; entry_num<num_colors; entry_num++) {
            char* entry = next_xpm_data(stream) ;
            if(entry == nil) {
                Logger::err("Image") 
                    << "XPM Image reader: Unexpected end of file" 
                    << std::endl ;
                delete colormap ;
                return nil ;
            }
     
            int key1 = entry[0] ;
            int key2 = entry[1] ;
      
            char* colorcode = (char*)strstr(entry, "c #") ;
            if(colorcode == NULL) {
                if(strstr(entry, "None") != NULL) {
                    colorcode = (char*)"c #000000" ;
                } else {
                    Logger::err("Image") 
                       << "XPM Image reader: Colormap entry without any color" 
                       << std::endl ;
                    Logger::err("Image") 
                        << "   entry = \'" << entry << "\'" << std::endl ;
                    return nil ;
                }
            }
        
            colorcode += 3 ;
      
            byte r = byte(16 * htoi(colorcode[0]) + htoi(colorcode[1])) ;
            byte g = byte(16 * htoi(colorcode[2]) + htoi(colorcode[3])) ;
            byte b = byte(16 * htoi(colorcode[4]) + htoi(colorcode[5])) ;      
            
            colormap-> color_cell(entry_num) = Colormap::ColorCell(r,g,b,255) ;
            conv_table[key1][key2] = (unsigned char)entry_num ;
        }
        
        // _______________________ image
        
        Image* result = new Image(Image::INDEXED, width, height) ;
        result-> set_colormap(colormap) ;
    
        for(int y=0; y<height; y++) {
            char* scan_line = next_xpm_data(stream) ;
            if(scan_line == nil) {
                Logger::err("Image") 
                    << "XPM Image reader: Unexpected end of file"
                    << std::endl ;
                delete result ;
                return nil ;
            }
            for(int x=0; x<width; x++) {
                int key1 = scan_line[2*x] ;
                int key2 = scan_line[2*x+1] ;
                int color_index = conv_table[key1][key2] ;
                if(color_index < 0 || color_index > num_colors) {
                    Logger::err("Image") 
                        << "XPM Image reader: Invalid color index in image" 
                        << std::endl ;
                    delete result ;
                    return nil ;
                }
                result-> base_mem()[y * width + x] = byte(color_index) ;
            }
        }
    
        flip_image(*result) ;
        return result ;
    }


    Image* ImageSerializer_xpm::read_xpm_1_byte_per_pixel(        
        int width, int height, int num_colors, std::istream& stream
    ) {
        
        // Converts a two-digit XPM color code into
        //  a color index.
        static int conv_table[256] ;

        // For checking, put a negative value to
        //  detect invalid color codes.
        for(int k1=0; k1 < 256; k1++) {
            conv_table[k1] = -1 ;
        }
        
        // _______________________  colormap
        
        typedef Numeric::uint8 byte ;
        
        Colormap* colormap = new Colormap(num_colors) ;
        
        for(int entry_num=0; entry_num<num_colors; entry_num++) {
            char* entry = next_xpm_data(stream) ;
            if(entry == nil) {
                Logger::err("Image") 
                    << "XPM Image reader: Unexpected end of file" 
                    << std::endl ;
                delete colormap ;
                return nil ;
            }
            
            int key1 = entry[0] ;
      
            char* colorcode = (char*)strstr(entry, "c #") ;
            if(colorcode == NULL) {
                if(strstr(entry, "None") != NULL) {
                    colorcode = (char*)"c #000000" ;
                } else {
                    Logger::err("Image") 
                       << "XPM Image reader: Colormap entry without any color" 
                        << std::endl ;
                    Logger::err("Image") 
                        << "   entry = \'" << entry << "\'" << std::endl ;
                    return nil ;
                }
            }
        
            colorcode += 3 ;
      
            byte r = byte(16 * htoi(colorcode[0]) + htoi(colorcode[1])) ;
            byte g = byte(16 * htoi(colorcode[2]) + htoi(colorcode[3])) ;
            byte b = byte(16 * htoi(colorcode[4]) + htoi(colorcode[5])) ;      
            
            colormap-> color_cell(entry_num) = Colormap::ColorCell(r,g,b,255) ;
            conv_table[key1] = (unsigned char)entry_num ;
        }
        
        // _______________________ image
        
        Image* result = new Image(Image::INDEXED, width, height) ;
        result-> set_colormap(colormap) ;
    
        for(int y=0; y<height; y++) {
            char* scan_line = next_xpm_data(stream) ;
            if(scan_line == nil) {
                Logger::err("Image") 
                    << "XPM Image reader: Unexpected end of file"
                    << std::endl ;
                delete result ;
                return nil ;
            }
            for(int x=0; x<width; x++) {
                int key1 = scan_line[x] ;
                int color_index = conv_table[key1] ;
                if(color_index < 0 || color_index > num_colors) {
                    Logger::err("Image") 
                        << "XPM Image reader: Invalid color index in image" 
                        << std::endl ;
                    delete result ;
                    return nil ;
                }
                result-> base_mem()[y * width + x] = byte(color_index) ;
            }
        }
    
        return result ;
    }

    bool ImageSerializer_xpm::binary() const {
        return false ;
    }

    char* ImageSerializer_xpm::next_xpm_data(std::istream& input) {
        static char line_buffer[4096] ;
        char* result = nil ;
        bool found = false ;
        while(!found && !input.eof()) {
            input.getline(line_buffer,4096) ;
            char* p1 = strchr(line_buffer,'\"') ;
            char* p2 = strchr(line_buffer + 1, '\"') ;
            found = (p1 != nil && p2 != nil) ;
            if(found) {
                result = p1 + 1 ;
                *p2 = '\0' ;
            }
        }
        return result ;
    }

    bool ImageSerializer_xpm::read_supported() const {
        return true ;
    }
