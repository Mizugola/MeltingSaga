#include <vili/node.hpp>

namespace obe::Config::Validators
{
    vili::node MountValidator()
    {
        // clang-format off
        return vili::object {
            {
                "Mount", vili::object {
                    {"type", vili::object_typename},
                    {
                        "items", vili::object {
                            {"type", vili::object_typename},
                            {
                                "properties", vili::object {
                                    {
                                        "type", vili::object {
                                            {"type", vili::string_typename},
                                            {"values", vili::array {"Path", "Workspace", "Package"}}
                                        },
                                    },
                                    {
                                        "path", vili::object {
                                            {"type", vili::string_typename}
                                        }
                                    },
                                    {
                                        "prefix", vili::object {
                                            {"type", vili::string_typename},
                                            {"optional", true}
                                        }
                                    },
                                    {
                                        "priority", vili::object {
                                            {"type", vili::integer_typename},
                                            {"min", 0},
                                            {"optional", true}
                                        }
                                    },
                                    {
                                        "implicit", vili::object {
                                            {"type", vili::boolean_typename},
                                            {"optional", true}
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        };
        // clang-format on
    }
}