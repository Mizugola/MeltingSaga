#pragma once

#include <sol/sol.hpp>
#include <vili/Vili.hpp>

/**
 * \brief Functions that allow the conversion of Vili elements to Lua elements
 */
namespace obe::Script::DataBridge
{
    void dataToLua(sol::table target, vili::Node* convert);
    vili::Node* luaToData(sol::reference convert);
    /**
     * \brief Adds a converted Vili ComplexAttribute to a Lua Table
     *        The key of the newly converted element in the table will be the
     *        ComplexAttribute's id
     * \param target The Lua Table you want to add the
     *        converted ComplexAttribute
     * \param convert The Vili ComplexAttribute you
     *        want to get the data from
     */
    void complexNodeToLuaTable(sol::table target, vili::ComplexNode* convert);
    /**
     * \brief Add a Vili BaseAttribute in a Lua Table
     *        The key of the newly converted element in the table will be the
     *        BaseAttribute's id
     * \param target The Lua Table you want to write the Vili
     *        element in
     * \param convert The Vili BaseAttribute you want to add in the
     *        Lua Table
     */
    void dataNodeToLuaElement(sol::table target, vili::DataNode* convert);
    /**
     * \brief Adds a converted Vili ListAttribute to a Lua Table.
     *        Index of the List will start at 0 (unlike default Lua lists which
     *        starts at 1). The key of the newly converted element in the table will
     *        be the ListAttribute's id
     * \param target The Lua Table you want to add the
     *        converted ListAttribute
     * \param convert The Vili ComplexAttribute you want
     *        to get the data from
     */
    void arrayNodeToLuaTable(sol::table target, vili::ArrayNode* convert);
    /**
     * \brief Converts a Lua Table to a Vili ComplexAttribute
     * \param id Id of the newly created ComplexAttribute
     * \param convert The LuaTable you want to convert to a ComplexAttribute
     * \return The converted Lua Table in ComplexAttribute form
     */
    vili::ComplexNode* luaTableToComplexNode(const std::string& id, sol::object convert);
    /**
     * \brief Converts a Lua Variable to a Vili BaseAttribute
     * \param id Id of the newly created BaseAttribute
     * \param convert The LuaVar you want to convert to a BaseAttribute
     * \return The converted Lua Element in BaseAttribute form
     */
    vili::DataNode* luaElementToDataNode(const std::string& id, sol::object convert);
    /**
     * \brief Converts a Lua Table to a Vili ListAttribute
     * \param id Id of the newly created ListAttribute
     * \param convert The Lua Table you want to convert to a ListAttribute
     * \return The converted Lua Table in ListAttribute form
     */
    vili::ArrayNode* luaTableToArrayNode(const std::string& id, sol::table convert);
} // namespace obe::Script::DataBridge