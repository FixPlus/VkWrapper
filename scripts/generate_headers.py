import xml.etree.ElementTree as xmlReader
import argparse


def reformatExtensionName(name):
    name_len = len(name)
    name = name.capitalize()
    for i in range(0, name_len - 1):
        if name[i] == '_':
            char = name[i + 1].upper()
            name = name[:i + 1] + char + name[i + 2:]
    name = name.replace("_", "")
    return name


def generateExtension(extension):
    extension_name = extension.attrib.get('name')
    extension_name = reformatExtensionName(extension_name)

    class_header = "class " + extension_name + ": public "
    is_instance_ext = extension.attrib.get('type') == "instance"

    if is_instance_ext:
        base_class = "InstanceExtensionBase"
        parent_type = "VkInstance"
        parent_var = "instance"
        initialize_type = "InstanceExtensionInitializerImpl"
    else:
        base_class = "DeviceExtensionBase"
        parent_type = "VkDevice"
        parent_var = "device"
        initialize_type = "DeviceExtensionInitializerImpl"

    tab = "   "

    class_header += base_class
    class_header += " {\n"
    class_header += "public:\n" + tab
    class_header += extension_name + "(PFN_getProcAddr getProcAddr, "
    class_header += parent_type + " " + parent_var

    class_header += " ) :\n" + tab

    ext_commands = []

    for command in extension[0].findall('command'):
        ext_commands.append(command.attrib.get('name'))

    for command in ext_commands:
        class_header += command + "(getProcAddrT<PFN_" + command + ">(\"" + command + "\")),\n" + tab

    class_header += base_class + "(getProcAddr, " + parent_var + ", \"" + extension.attrib.get('name') + "\")\n" + tab
    class_header += "{};\n"

    for command in ext_commands:
        class_header += tab + "PFN_" + command + " " + command + ";\n"

    class_header += "constexpr static const " + initialize_type + "<" + extension_name + "> m_initializer{};\n"
    class_header += "};\n"
    return class_header


def generateExtensionDefinitions():
    docRoot = doc.getroot()


    instance_ext_name_list = []
    device_ext_name_list = []

    for extension in docRoot.find('extensions'):
        if len(extension) == 0 or extension.attrib.get('supported') != "vulkan":
            continue
        if extension.attrib.get('platform') is not None and extension.attrib.get('platform') != platform:
            continue
        print(generateExtension(extension))
        if extension.attrib.get('type') == 'instance':
            instance_ext_name_list.append(extension.attrib.get('name'))
        else:
            device_ext_name_list.append(extension.attrib.get('name'))

    print("const std::unordered_map < std::string, InstanceExtensionInitializerBase const * >")
    print(" m_instanceExtInitializers {")

    for extension in instance_ext_name_list:
        print("{\"" + extension + "\", &" + reformatExtensionName(extension) + "::m_initializer},")

    print('};')

    print("const std::unordered_map < std::string, DeviceExtensionInitializerBase const * >")
    print(" m_deviceExtInitializers {")

    for extension in device_ext_name_list:
        print("{\"" + extension + "\", &" + reformatExtensionName(extension) + "::m_initializer},")

    print('};')
    return


global_command_list = [
    'vkCreateInstance',
    'vkEnumerateInstanceExtensionProperties',
    'vkEnumerateInstanceLayerProperties',
    'vkGetInstanceProcAddr',
    'vkEnumerateInstanceVersion'
]


def globalCommand(command):
    for glob in global_command_list:
        if command == glob:
            return True


def instanceLevelCommand(command):
    # exceptional case
    if command == 'vkGetDeviceProcAddr':
        return True
    commands = doc.getroot().find('commands')
    for com_desc in commands.findall('command'):
        proto = com_desc.find('proto')
        if proto is None:
            continue
        if proto.find('name').text != command:
            continue
        first_param_type = com_desc.find('param').find('type').text
        if first_param_type == 'VkInstance' or first_param_type == 'VkPhysicalDevice':
            return True
        else:
            return False
    return True


def generateCoreDefinitions():
    feature_apis = doc.getroot().findall('feature')
    minor = 0
    major = 1
    prev_instance_class = "SymbolTableBase<VkInstance>"
    prev_device_class = "SymbolTableBase<VkDevice>"

    for feature in feature_apis:
        if feature.attrib.get('number') != str(major) + '.' + str(minor):
            major += 1
            minor = 0
            continue

        current_instance_class = "InstanceCore<" + str(major) + "," + str(minor) + ">"
        instance_core_header = "template<>\n"
        instance_core_header += "class " + current_instance_class + ": public " + prev_instance_class
        instance_core_header += "{\npublic:\n"
        instance_core_header += "InstanceCore(PFN_getProcAddr getProcAddr, VkInstance instance) :\n"

        current_device_class = "DeviceCore<" + str(major) + "," + str(minor) + ">"
        device_core_header = "template<>\n"
        device_core_header += "class " + current_device_class + " : public " + prev_device_class
        device_core_header += "{\npublic:\n"
        device_core_header += "DeviceCore(PFN_getProcAddr getProcAddr, VkDevice device) :\n"

        instance_command_list = []
        device_command_list = []
        for command in feature.iter('command'):
            command_name = command.attrib.get('name')
            if globalCommand(command_name):
                continue
            if instanceLevelCommand(command_name):
                instance_command_list.append(command_name)
            else:
                device_command_list.append(command_name)

        for command in instance_command_list:
            instance_core_header += command + "(getProcAddrT<PFN_" + command + ">(\"" + command + "\")),\n"
        for command in device_command_list:
            device_core_header += command + "(getProcAddrT<PFN_" + command + ">(\"" + command + "\")),\n"

        instance_core_header += prev_instance_class + "(getProcAddr, instance)\n"
        instance_core_header += "{};\n"

        device_core_header += prev_device_class + "(getProcAddr, device)\n"
        device_core_header += "{};\n"

        for command in instance_command_list:
            instance_core_header += "PFN_" + command + " " + command + ";\n"
        for command in device_command_list:
            device_core_header += "PFN_" + command + " " + command + ";\n"

        instance_core_header += "};\n"
        device_core_header += "};\n"

        print(instance_core_header)
        print(device_core_header)
        prev_instance_class = current_instance_class
        prev_device_class = current_device_class

        minor += 1


if __name__ == '__main__':
    args = argparse.ArgumentParser()

    args.add_argument('-platform', action='store', default='win32',
                      help='only those platform-specific extensions with will be built')

    args.add_argument('-path', action='store', default='.',
                      help='path to vk.xml')

    parsed_args = args.parse_args()

    platform = parsed_args.platform

    doc = xmlReader.parse(parsed_args.path + '/vk.xml')

    generateCoreDefinitions()
    generateExtensionDefinitions()














