def bin_to_c_array(bin_file, header_file, array_name):
    with open(bin_file, 'rb') as f:
        data = f.read()

    with open(header_file, 'w') as f:
        f.write('#include <stdint.h>\n\n')
        f.write('uint8_t {}[] = {{\n'.format(array_name))
        for i, byte in enumerate(data):
            f.write('0x{:02x}, '.format(byte))
            if i % 16 == 15:
                f.write('\n')
        f.write('\n};\n')

# Usage
bin_to_c_array('bios', 'bios.h', 'bios')