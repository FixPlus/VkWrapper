import xml.etree.ElementTree as xmlReader
import argparse

class VulkanExtension:
    def __init__(self, extension_body, extension_name):
        self.body = extension_body
        self.name = reformatExtensionName(extension_name)
        self.string_name = extension_name


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
    return VulkanExtension(class_header, extension.attrib.get('name'))


def generateExtensionDefinitions():
    docRoot = doc.getroot()
    
    platform_macro_map = { platform.attrib.get('name'): platform.attrib.get('protect') for platform in docRoot.find('platforms').findall('platform')}
 
    instance_ext_list ={ platform.attrib.get('name'): [] for platform in docRoot.find('platforms').findall('platform')}
    instance_ext_list['base'] = []
    device_ext_list = { platform.attrib.get('name'): [] for platform in docRoot.find('platforms').findall('platform')}
    device_ext_list['base'] = []

    for extension in docRoot.find('extensions'):
        if len(extension) == 0 or extension.attrib.get('supported') != "vulkan":
            continue
        platform = extension.attrib.get('platform')
        if(platform is None):
           platform = 'base'
           
        if extension.attrib.get('type') == 'instance':
            instance_ext_list[platform].append(generateExtension(extension))
        else:
            device_ext_list[platform].append(generateExtension(extension))

    print("#ifdef VKW_DUMP_EXTENSION_CLASSES")

    for ext_platform in instance_ext_list.keys():
        if len(instance_ext_list[ext_platform]) == 0:
            continue
        insert_protect = ext_platform != 'base'
        if insert_protect:
            print("#ifdef " + platform_macro_map[ext_platform])
        for ext in instance_ext_list[ext_platform]:
            print(ext.body)
        if insert_protect:
            print("#endif")
            
    for ext_platform in device_ext_list.keys():
        if len(device_ext_list[ext_platform]) == 0:
            continue
        insert_protect = ext_platform != 'base'
        if insert_protect:
            print("#ifdef " + platform_macro_map[ext_platform])
        for ext in device_ext_list[ext_platform]:
            print(ext.body)
        if insert_protect:
            print("#endif")

    print("#endif") # #ifdef VKW_DUMP_EXTENSION_CLASSES

    print("#ifdef VKW_DUMP_EXTENSION_INITIALIZERS_MAP_DEFINITION")
    for ext_platform in instance_ext_list.keys():
        if len(instance_ext_list[ext_platform]) == 0:
            continue
        insert_protect = ext_platform != 'base'
        if insert_protect:
            print("#ifdef " + platform_macro_map[ext_platform])
        for ext in instance_ext_list[ext_platform]:
            print("VKW_INSTANCE_MAP_ENTRY(\"" + ext.string_name + "\", &" + ext.name + "::m_initializer)")
        if insert_protect:
            print("#endif")

    for ext_platform in device_ext_list.keys():
        if len(device_ext_list[ext_platform]) == 0:
            continue
        insert_protect = ext_platform != 'base'
        if insert_protect:
            print("#ifdef " + platform_macro_map[ext_platform])
        for ext in device_ext_list[ext_platform]:
            print("VKW_DEVICE_MAP_ENTRY(\"" + ext.string_name + "\", &" + ext.name + "::m_initializer)")
        if insert_protect:
            print("#endif")
    print("#endif")
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

    print("#ifdef VKW_DUMP_CORE_CLASSES")

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
    print("#endif") # #ifdef VKW_DUMP_CORE_CLASSES

if __name__ == '__main__':
    args = argparse.ArgumentParser()

    args.add_argument('-path', action='store', default='.',
                      help='path to vk.xml')

    parsed_args = args.parse_args()

    doc = xmlReader.parse(parsed_args.path + '/vk.xml')

    generateCoreDefinitions()
    generateExtensionDefinitions()














