import xml.etree.ElementTree as xmlReader
import argparse


def generate_feature_class(feature):
    feature_name = feature.find('name').text
    vk_bool_type = feature.find('type').text
    class_header = "class " + feature_name + ": public FeatureBase {\n"
    class_header += "public:\n"
    class_header += "  " + vk_bool_type + "* feature_location(VkPhysicalDeviceFeatures* featureList) const override {\n"
    class_header += "     return reinterpret_cast<" + vk_bool_type + \
                    "*>(reinterpret_cast<char*>(featureList) + offsetof(VkPhysicalDeviceFeatures, " \
                    + feature_name + "));\n"
    class_header += "   };\n"
    class_header += "  " + vk_bool_type + " const* feature_location(VkPhysicalDeviceFeatures const" + \
                    "* featureList) const override {\n"
    class_header += "     return reinterpret_cast<const " + vk_bool_type + \
                    "*>(reinterpret_cast<const char*>(featureList) + offsetof(VkPhysicalDeviceFeatures, " \
                    + feature_name + "));\n"
    class_header += "   };\n"
    class_header += "   const char* name() const override {\n"
    class_header += "      return m_name;\n"
    class_header += "   };\n"
    class_header += "private:\n"
    class_header += "   constexpr static const char* m_name = \"" + feature_name + "\";\n"
    class_header += "};\n"
    return class_header


if __name__ == '__main__':
    args = argparse.ArgumentParser()

    args.add_argument('-path', action='store', default='.',
                      help='path to vk.xml')

    parsed_args = args.parse_args()

    doc = xmlReader.parse(parsed_args.path + '/vk.xml')

    output = ""
    for vk_type in doc.getroot().find('types').findall('type'):
        if vk_type.attrib.get('name') != 'VkPhysicalDeviceFeatures':
            continue
        for feature in vk_type.findall('member'):
            output += generate_feature_class(feature)
        break

    print(output)