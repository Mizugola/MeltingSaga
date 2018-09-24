local Class = require("Lib/StdLib/Class");
local contains = require("Lib/StdLib/Contains");

obe.Canvas.Canvas = Class("Canvas", function(self, width, height, usecache)
    self.internal = obe.Canvas.InternalCanvas(width, height);
    self.elements = {};
    self.useCache = usecache or false;
end);

function obe.Canvas.NormalizeColor(color, base)
    if type(color) == "table" then
        local ncolor = SFML.Color();
        base = base or { r = 0, g = 0, b = 0, a = 255 };
        ncolor.r = color.r or base.r;
        ncolor.g = color.g or base.g;
        ncolor.b = color.b or base.b;
        ncolor.a = color.a or base.a;
        return ncolor;
    elseif type(color) == "number" then
        local dalpha = base.a;
        return SFML.Color(color, color, color, dalpha);
    elseif type(color) == "string" then
        color = color:gsub("#","");
        if string.len(color) == 3 then
            return SFML.Color(tonumber("0x"..color:sub(1,1)) * 17, tonumber("0x"..color:sub(2,2)) * 17, tonumber("0x"..color:sub(3,3)) * 17);
        elseif string.len(color) == 6 then
            return SFML.Color(tonumber("0x"..color:sub(1,2)), tonumber("0x"..color:sub(3,4)), tonumber("0x"..color:sub(5,6)));
        end
    end
end

function obe.Canvas.ConvertHAlign(align)
    if type(align) == "string" then
        if align == "Left" then return obe.Canvas.Alignment.Horizontal.Left;
        elseif align == "Center" then return obe.Canvas.Alignment.Horizontal.Center;
        elseif align == "Right" then return obe.Canvas.Alignment.Horizontal.Right;
        end
    else
        if align == obe.Canvas.Alignment.Horizontal.Left then return "Left";
        elseif align == obe.Canvas.Alignment.Horizontal.Center then return "Center";
        elseif align == obe.Canvas.Alignment.Horizontal.Right then return "Right";
        end
    end
end

function obe.Canvas.ConvertVAlign(align)
    if type(align) == "string" then
        if align == "Top" then return obe.Canvas.Alignment.Vertical.Top;
        elseif align == "Center" then return obe.Canvas.Alignment.Vertical.Center;
        elseif align == "Bottom" then return obe.Canvas.Alignment.Vertical.Bottom;
        end
    else
        if align == obe.Canvas.Alignment.Vertical.Top then return "Top";
        elseif align == obe.Canvas.Alignment.Vertical.Center then return "Center";
        elseif align == obe.Canvas.Alignment.Vertical.Bottom then return "Bottom";
        end
    end
end

function deepcopy(value)
    local orig_type = type(value);
    local copy;
    if orig_type == 'table' then
        copy = {};
        for orig_key, orig_value in next, value, nil do
            copy[deepcopy(orig_key)] = deepcopy(orig_value);
        end
    else
        copy = value;
    end
    return copy;
end

obe.Canvas.Bases = {};

function obe.Canvas.ApplySetterValue(internal, setters, key, value)
    print("Applying Setter Value", key, value);
    if type(key) == "number" and not setters[key] and setters.__number then
        setters.__number(internal, key, value);
    else
        if setters[key] then
            setters[key](internal, value);
        else
            print("Can't find obe.Canvas.Canvas attribute : " .. tostring(key));
        end
    end
end

function obe.Canvas.ApplyCache(element)
    --print("Applying cache on", inspect(element));
    local applyCacheValue = function(mt, k, v)
        if type(k) == "number" and not mt.__setters[k] and mt.__setters.__number then
            mt.__setters.__number(mt.__ref, k, v);
        else
            if mt.__setters[k] then
                mt.__setters[k](mt.__ref, v);
            else
                error("Can't find obe.Canvas.Canvas attribute : " .. tostring(k));
            end
        end
    end
    local mt = getmetatable(element);
    --print("Cache :", inspect(mt.__cache));
    if #mt.__priority > 0 then
        for k, v in pairs(mt.__priority) do
            if mt.__cache[v] then
                applyCacheValue(mt, v, mt.__cache[v]);
                mt.__cache[v] = nil;
            end
        end
    end
    for k, v in pairs(mt.__cache) do
        applyCacheValue(mt, k, v);
    end
    for k, v in pairs(mt.__getters) do
        if type(v) == "table" then
            obe.Canvas.ApplyCache(v);
        end
    end
    mt.__cache = {};
end

function obe.Canvas.__set_cached(tbl, key, value)
    local k = getmetatable(tbl);
    if type(value) == "table" then
        k.__getters[key](value);
    else
        k.__cache[key] = value;
        k.__rcache[key] = value;
    end
end

function obe.Canvas.__get_cached(tbl, key)
    local k = getmetatable(tbl);
    if k.__rcache[key] and type(k.__rcache[key]) ~= "table" then
        return k.__rcache[key];
    else
        if type(k.__getters[key]) == "function" then
            return k.__getters[key](k.__ref);
        elseif type(key) == "number" and not k.__getters[key] and k.__getters.__number then
            return k.__getters.__number(k.__ref, key);
        elseif type(k.__getters[key]) == "table" then
            --print("Found table", inspect(k.__getters[key]));
            return k.__getters[key];
        end
    end
end

function obe.Canvas.__set(tbl, key, value)
    local k = getmetatable(tbl);
    if type(key) == "number" and not k.__setters[key] and k.__setters.__number then
        k.__setters.__number(k.__ref, value);
    else
        if k.__setters[key] then
            k.__setters[key](k.__ref, value);
        else
            error("Can't find obe.Canvas.Canvas attribute : " .. tostring(key));
        end
    end
end

function obe.Canvas.__get(tbl, key)
    local k = getmetatable(tbl);
    if type(k.__getters[key]) == "function" then
        return k.__getters[key](k.__ref);
    elseif type(key) == "number" and not k.__getters[key] and k.__getters.__number then
        return k.__getters.__number(k.__ref);
    elseif type(k.__getters[key]) == "table" then
        return k.__getters[key];
    end
end

function obe.Canvas.__call(tbl, values)
    values = values or {};
    for k, v in pairs(values) do
        tbl[k] = v;
    end
    return tbl;
end

function obe.Canvas.OldMakeMT(bases, usecache)
    usecache = usecache or false;
    local getters = {};
    local setters = {};
    local priority = {};

    local tt = { __type = "CanvasMT" };

    local tAccess = function(a, b)
        b = b or {};
        local k = getmetatable(a);
        for k, v in pairs(b) do
            a[k] = v;
        end
        return a;
    end
    for kb, base in pairs(bases) do
        if base.priority then
            for k, priorityName in pairs(base.priority) do
                if not contains(priority, priorityName) then
                    table.insert(priority, priorityName);
                end
            end
        end
        for getterName, getterValue in pairs(base.getters) do
            getters[getterName] = getterValue;
        end
        for setterName, setterValue in pairs(base.setters) do
            setters[setterName] = setterValue;
        end
    end
    for key, getter in pairs(getters) do
        if type(getter) == "table" then
            getters[key] = obe.Canvas.MakeMT({getter}, usecache);
        end
    end
    for key, setter in pairs(setters) do
        if type(setter) == "table" then
            setters[key] = obe.Canvas.MakeMT({setter}, usecache);
        end
    end
    local getfunc = usecache and obe.Canvas.__get_cached or obe.Canvas.__get;
    local setfunc = usecache and obe.Canvas.__set_cached or obe.Canvas.__set;
    local mt = {
        __ref = nil,
        __getters = getters,
        __setters = setters,
        __priority = priority,
        __index = getfunc,
        __newindex = setfunc,
        __call = tAccess,
        __cache = {},
        __rcache = {}
    };
    setmetatable(tt, mt);
    return tt;
end

function obe.Canvas.MakeMT(bases)
    local getters = {};
    local setters = {};
    local priority = {};

    local tt = { __type = "CanvasMT" };

    for kb, base in pairs(bases) do
        if base.priority then
            for k, priorityName in pairs(base.priority) do
                if not contains(priority, priorityName) then
                    table.insert(priority, priorityName);
                end
            end
        end
        for getterName, getterValue in pairs(base.getters) do
            getters[getterName] = getterValue;
        end
        for setterName, setterValue in pairs(base.setters) do
            setters[setterName] = setterValue;
        end
    end
    for key, getter in pairs(getters) do
        if type(getter) == "table" then
            getters[key] = obe.Canvas.MakeMT({getter});
        end
    end
    for key, setter in pairs(setters) do
        if type(setter) == "table" then
            setters[key] = obe.Canvas.MakeMT({setter});
        end
    end
    local mt = {
        __getters = getters,
        __setters = setters,
        __priority = priority,
        __index = obe.Canvas.__get,
        __newindex = obe.Canvas.__set,
        __call = obe.Canvas.__call,
    };
    setmetatable(tt, mt);
    return tt;
end

function obe.Canvas.__instance_index(tbl, key)
    local mt = getmetatable(tbl);
    return mt.__values[key];
end

function obe.Canvas.__instance_newindex(tbl, key, value)
    local mt = getmetatable(tbl);
    if type(value) == "table" then
        rawset(tbl, key, {});
        tbl[key](value);
    else
        mt.__values[key] = value;
    end
end

function obe.Canvas.__instance_call(tbl, values)
    values = values or {};
    for k, v in pairs(values) do
        tbl[k] = v;
    end
    return tbl;
end

function obe.Canvas.InstanciateMT(type, internal)
    local instance = {};
    local instanceMT = {
        __type = type,
        __ref = internal,
        __call = obe.Canvas.__instance_call,
        __cache = {}
    };
    setmetatable(instance, instanceMT);
    obe.Canvas.FillInstanceValues(obe.Canvas.BMT[type], instance, internal);
    return instance;
end

function obe.Canvas.FillInstanceValues(base, instance, internal)
    local mt = getmetatable(base);
    for k, v in pairs(mt.__getters) do
        if type(v) == "function" then
            instance[k] = v(internal);
        elseif type(v) == "table" then
            instance[k] = {};
            obe.Canvas.FillInstanceValues(v, instance[k], internal)
        end
    end
end

function obe.Canvas.InjectInternalMT(tbl, ref)
    if type(tbl) == "table" and rawget(tbl, "__type") == "CanvasMT" then
        local mt = getmetatable(tbl);
        mt.__ref = ref;
        for k, getter in pairs(mt.__getters) do
            obe.Canvas.InitializeMT(getter, ref);
        end
        for k, setter in pairs(mt.__setters) do
            obe.Canvas.InitializeMT(setter, ref);
        end
    end
end

local UV2V2f = function(uv)
    local uvpx = uv:to(obe.Units.ScenePixels);
    return SFML.Vector2f(uvpx.x, uvpx.y);
end

obe.Canvas.Bases.Drawable = {
  getters = {
      layer = function(self) return self.layer; end,
      visible = function(self) return self.visible; end,
      id = function(self) return self.id; end
  },
  setters = {
      layer = function(self, layer) self:setLayer(layer or 1); end,
      visible = function(self, visible) self.visible = visible; end
  }
}

obe.Canvas.Bases.Line = {
    getters = {
        p1 = {
            getters = {
                x = function(self) return self.p1.x; end,
                y = function(self) return self.p1.y; end,
                unit = function(self) return self.p1.unit; end,
                color = {
                    getters = {
                        r = function(self) return self.p1color.r; end,
                        g = function(self) return self.p1color.g; end,
                        b = function(self) return self.p1color.b; end,
                        a = function(self) return self.p1color.a; end
                    },
                    setters = {
                        r = function(self, r) self.p1color.r = r or 0; end,
                        g = function(self, g) self.p1color.g = g or 0; end,
                        b = function(self, b) self.p1color.b = b or 0; end,
                        a = function(self, a) self.p1color.a = a or 255; end
                    }
                },
            },
            setters = {
                x = function(self, x) self.p1.x = x or 0; end,
                y = function(self, y) self.p1.y = y or 0; end,
                unit = function(self, unit)
                    self.p1.unit = unit or obe.Units.ScenePixels;
                end,
                color = function(self, color)
                    self.p1color = obe.Canvas.NormalizeColor(color, self.p1color);
                end
            }
        },
        p2 = {
            getters = {
                x = function(self) return self.p2.x; end,
                y = function(self) return self.p2.y; end,
                unit = function(self) return self.p2.unit; end,
                color = {
                    getters = {
                        r = function(self) return self.p2color.r; end,
                        g = function(self) return self.p2color.g; end,
                        b = function(self) return self.p2color.b; end,
                        a = function(self) return self.p2color.a; end
                    },
                    setters = {
                        r = function(self, r) self.p2color.r = r or 0; end,
                        g = function(self, g) self.p2color.g = g or 0; end,
                        b = function(self, b) self.p2color.b = b or 0; end,
                        a = function(self, a) self.p2color.a = a or 255; end
                    }
                }
            },
            setters = {
                x = function(self, x) self.p2.x = x or 0; end,
                y = function(self, y) self.p2.y = y or 0; end,
                unit = function(self, unit)
                    self.p2.unit = unit or obe.Units.ScenePixels;
                end,
                color = function(self, color)
                    self.p2color = obe.Canvas.NormalizeColor(color, self.p2color);
                end
            }
        },
        unit = function(self) return self.p1.unit; end,
        thickness = function(self) return self.thickness; end,
        color = {
            getters = {
                r = function(self) return self.p1color.r; end,
                g = function(self) return self.p1color.g; end,
                b = function(self) return self.p1color.b; end,
                a = function(self) return self.p1color.a; end
            },
            setters = {
                r = function(self, r) self.p1color.r = r or 0; self.p2color.r = r or 0; end,
                g = function(self, g) self.p1color.g = g or 0; self.p2color.g = g or 0; end,
                b = function(self, b) self.p1color.b = b or 0; self.p2color.b = b or 0; end,
                a = function(self, a) self.p1color.a = a or 255; self.p2color.a = a or 255; end
            }
        }
    },
    setters = {
        unit = function(self, unit)
            self.p1.unit = unit or obe.Units.ScenePixels;
            self.p2.unit = unit or obe.Units.ScenePixels;
        end,
        thickness = function(self, thickness) self.thickness = thickness or 1; end,
        color = function(self, color)
            self.p1color = obe.Canvas.NormalizeColor(color, self.p1color);
            self.p2color = obe.Canvas.NormalizeColor(color, self.p2color);
        end,
        p1 = function(self, p1)
            if type(p1) == "table" then
                if type(p1.x) == "number" then self.p1.x = p1.x; end
                if type(p1.y) == "number" then self.p1.y = p1.y; end
                if p1.color then self.p1color = obe.Canvas.NormalizeColor(p1.color, self.p1color); end
            end
        end,
        p2 = function(self, p2)
            if type(p2) == "table" then
                if type(p2.x) == "number" then self.p2.x = p2.x; end
                if type(p2.y) == "number" then self.p2.y = p2.y; end
                if p2.color then self.p2color = obe.Canvas.NormalizeColor(p2.color, self.p2color); end
            end
        end
    }
};

obe.Canvas.Bases.Shape = {
    priority = {
        "unit"
    },
    getters = {
        x = function(self) return self.position.x; end,
        y = function(self) return self.position.y; end,
        unit = function(self) return self.position.unit; end,
        angle = function(self) return self.shape:getRotation(); end,
        scale = {
            getters = {
                x = function(self) return self.shape:getScale().x; end,
                y = function(self) return self.shape:getScale().y; end
            },
            setters = {
                x = function(self, x)
                    local y = self.shape:getScale().y;
                    self.shape:setScale(x, y);
                end,
                y = function(self, y)
                    local x = self.shape:getScale().x;
                    self.shape:setScale(x, y);
                end
            }
        },
        outline = {
            getters = {
                color = {
                    getters = {
                        r = function(self) return self.shape:getOutlineColor().r; end,
                        g = function(self) return self.shape:getOutlineColor().g; end,
                        b = function(self) return self.shape:getOutlineColor().b; end,
                        a = function(self) return self.shape:getOutlineColor().a; end
                    },
                    setters = {
                        r = function(self, r)
                            local colorBuffer = self.shape:getOutlineColor();
                            colorBuffer.r = r or 0;
                            self.shape:setOutlineColor(colorBuffer);
                        end,
                        g = function(self, g)
                            local colorBuffer = self.shape:getOutlineColor();
                            colorBuffer.g = g or 0;
                            self.shape:setOutlineColor(colorBuffer);
                        end,
                        b = function(self, b)
                            local colorBuffer = self.shape:getOutlineColor();
                            colorBuffer.b = b or 0;
                            self.shape:setOutlineColor(colorBuffer);
                        end,
                        a = function(self, a)
                            local colorBuffer = self.shape:getOutlineColor();
                            colorBuffer.a = a or 255;
                            self.shape:setOutlineColor(colorBuffer);
                        end
                    }
                },
                thickness = function(self) return self.shape:getOutlineThickness(); end
            },
            setters = {
                color = function(self, color)
                    self.shape:setOutlineColor(obe.Canvas.NormalizeColor(color, self.shape:getOutlineColor()));
                end,
                thickness = function(self, thickness) self.shape:setOutlineThickness(thickness or 1); end
            }
        },
        color = {
            getters = {
                r = function(self) return self.shape:getFillColor().r; end,
                g = function(self) return self.shape:getFillColor().g; end,
                b = function(self) return self.shape:getFillColor().b; end,
                a = function(self) return self.shape:getFillColor().a; end
            },
            setters = {
                r = function(self, r)
                    local colorBuffer = self.shape:getFillColor();
                    colorBuffer.r = r or 0;
                    self.shape:setFillColor(colorBuffer);
                end,
                g = function(self, g)
                    local colorBuffer = self.shape:getFillColor();
                    colorBuffer.g = g or 0;
                    self.shape:setFillColor(colorBuffer);
                end,
                b = function(self, b)
                    local colorBuffer = self.shape:getFillColor();
                    colorBuffer.b = b or 0;
                    self.shape:setFillColor(colorBuffer);
                end,
                a = function(self, a)
                    local colorBuffer = self.shape:getFillColor();
                    colorBuffer.a = a or 255;
                    self.shape:setFillColor(colorBuffer);
                end
            }
        }
    },
    setters = {
        outline = function(self, outline)
            if type(outline) == "table" then
                if outline.color then
                    self.shape:setOutlineColor(obe.Canvas.NormalizeColor(outline.color, self.shape:getOutlineColor()));
                end
                if type(outline.thickness) == "number" then
                    self.shape:setOutlineThickness(outline.thickness);
                end
            end
        end,
        color = function(self, color)
            self.shape:setFillColor(obe.Canvas.NormalizeColor(color, self.shape:getFillColor()));
        end,
        x = function(self, x)
            self.position.x = x;
            self.shape:setPosition(UV2V2f(self.position));
        end,
        y = function(self, y)
            self.position.y = y;
            self.shape:setPosition(UV2V2f(self.position));
        end,
        unit = function(self, unit)
            self.position.unit = unit or obe.Units.ScenePixels;
        end,
        angle = function(self, angle)
            self.shape:setRotation(angle or 0);
        end,
        scale = function(self, scale)
            if type(scale) == "number" then
                self.shape:setScale(scale, scale);
            elseif type(scale) == "table" then
                self.shape:setScale(scale.x or 1, scale.y or 1);
            end
        end
    }
}

obe.Canvas.Bases.Rectangle = {
    getters = {
        width = function(self) return self.size.x; end,
        height = function(self) return self.size.y; end
    },
    setters = {
        width = function(self, width)
            self.size.x = width;
            self.shape:setSize(UV2V2f(self.size));
        end,
        height = function(self, height)
            self.size.y = height
            self.shape:setSize(UV2V2f(self.size));
        end,
        unit = function(self, unit)
            self.position.unit = unit or obe.Units.ScenePixels;
            self.size.unit = unit or obe.Units.ScenePixels;
        end,
    }
}

obe.Canvas.Bases.Circle = {
    getters = {
        radius = function(self) return self.shape:getRadius(); end,
        width = function(self) return self.shape:getGlobalBounds().width; end,
        height = function(self) return self.shape:getGlobalBounds().height; end
    },
    setters = {
        radius = function(self, radius) return self.shape:setRadius(radius); end
    }
}

obe.Canvas.Bases.Polygon = {
    getters = {

    },
    setters = {
        __number = function(self, index, vertex)
            local i = index - 1;
        end
    }
}

function GetRichTextString(shape)
    local fullText = "";
    for _, line in pairs(shape:getLines()) do
        for _, text in pairs(line:getTexts()) do
            fullText = fullText .. text:getString():toAnsiString();
        end
        fullText = fullText .. "\n";
    end
    if fullText and fullText ~= "" then
        fullText = string.sub(fullText, 1, -2);
    end
    return fullText;
end

obe.Canvas.Bases.Text = {
    priority = {
        "font", "size", "color", "outline"
    },
    getters = {
        text = function(self)
            local fullText = "";
            for _, line in pairs(self.shape:getLines()) do
                for _, text in pairs(line:getTexts()) do
                    fullText = fullText .. text:getString():toAnsiString();
                end
                fullText = fullText .. "\n";
            end
            if fullText ~= "" then
                fullText = fullText:sub(1, -2);
            end
            return fullText;
        end,
        size = function(self)
            return self.shape:getCharacterSize();
        end,
        font = function(self)
            return self.fontPath;
        end,
        width = function(self)
            return self.shape:getGlobalBounds().width;
        end,
        height = function(self)
            return self.shape:getGlobalBounds().height;
        end,
        align = {
            getters = {
                h = function(self) return obe.Canvas.ConvertHAlign(self.h_align); end,
                horizontal = function(self) return obe.Canvas.ConvertHAlign(self.h_align); end,
                v = function(self) return obe.Canvas.ConvertVAlign(self.v_align); end,
                vertical = function(self) return obe.Canvas.ConvertVAlign(self.v_align); end
            },
            setters = {
                h = function(self, h) self.h_align = obe.Canvas.ConvertHAlign(h); end,
                horizontal = function(self, h) self.h_align = obe.Canvas.ConvertHAlign(h); end,
                v = function(self, v) self.v_align = obe.Canvas.ConvertVAlign(v); end,
                vertical = function(self, v) self.v_align = obe.Canvas.ConvertVAlign(v); end
            }
        }
    },
    setters = {
        text = function(self, text)
            self.shape:clear();
            -- Apply style
            if self.fontPath == "" then
                error("@Canvas.Text.setters.text : Need to set @font before @text");
            end
            self.shape:pushString(SFML.WString(text));
        end,
        size = function(self, size)
            self.shape:setCharacterSize(size);
        end,
        font = function(self, font)
            self.fontPath = font;
            self.shape:setFont(obe.ResourceManager.GetFont(font));
        end,
        color = function(self, color)
            self.shape:clear();
            self.shape:pushFillColor(obe.Canvas.NormalizeColor(color));
            self.shape:pushString(SFML.String(GetRichTextString(self.shape)));
        end,
        outline = function(self, outline)
            if type(outline) == "table" then
                if outline.color then
                    self.shape:clear();
                    self.shape:pushOutlineColor(obe.Canvas.NormalizeColor(outline.color));
                    self.shape:pushString(SFML.String(GetRichTextString(self.shape)));
                end
                if type(outline.thickness) == "number" then
                    self.shape:clear();
                    self.shape:pushOutlineThickness(outline.thickness);
                    self.shape:pushString(SFML.String(GetRichTextString(self.shape)));
                end
            end
        end,
        __number = function(self, index, part)
            self.shape:pushOutlineThickness(0);
            self.shape:pushOutlineColor(SFML.Color(255, 255, 255));
            self.shape:pushFillColor(SFML.Color(255, 255, 255));
            self.shape:pushStyle(SFML.Style.Regular);
            if part.color then
                self.shape:pushFillColor(obe.Canvas.NormalizeColor(part.color));
            end
            if part.style then
                self.shape:pushStyle(part.style);
            end
            if part.outline then
                if part.outline.thickness then
                    self.shape:pushOutlineThickness(part.outline.thickness);
                end
                if part.outline.color then
                    self.shape:pushOutlineColor(obe.Canvas.NormalizeColor(part.outline.color));
                end
            end
            if part.text then
                self.shape:pushString(SFML.String(part.text));
            end
        end,
        align = function(self, al)
            if al.h or al.horizontal then
                self.h_align = obe.Canvas.ConvertHAlign(al.h or al.horizontal);
            end
            if al.v or al.vertical then
                self.v_align = obe.Canvas.ConvertVAlign(al.v or al.vertical);
            end
        end
    }
}

obe.Canvas.BMT = {
    Line = obe.Canvas.MakeMT({obe.Canvas.Bases.Drawable, obe.Canvas.Bases.Line}),
    Rectangle = obe.Canvas.MakeMT({obe.Canvas.Bases.Drawable, obe.Canvas.Bases.Shape, obe.Canvas.Bases.Rectangle}),
    Text = obe.Canvas.MakeMT({obe.Canvas.Bases.Drawable, obe.Canvas.Bases.Shape, obe.Canvas.Bases.Text}),
    Circle = obe.Canvas.MakeMT({obe.Canvas.Bases.Drawable, obe.Canvas.Bases.Shape, obe.Canvas.Bases.Circle})
};

function obe.Canvas.Canvas:GenerateId(id)
    if type(id) == "string" and self.internal:get(id) ~= nil then
        error("CanvasElement '" .. tostring(id) .. "' already exists !");
    end
    while id == nil or self.internal:get(id) ~= nil do
        id = obe.String.getRandomKey(obe.String.Alphabet .. obe.String.Numbers, 12);
    end
    return id;
end

function obe.Canvas.Canvas:Line(id)
    id = self:GenerateId(id);
    self.elements[id] = obe.Canvas.InstanciateMT("Line", self.internal:Line(id));
    --print(inspect(self.elements[id]));
    return self.elements[id];
end

function obe.Canvas.Canvas:Rectangle(id)
    id = self:GenerateId(id);
    self.elements[id] = obe.Canvas.MakeMT({
        obe.Canvas.Bases.Drawable,
        obe.Canvas.Bases.Shape,
        obe.Canvas.Bases.Rectangle}, self.useCache);
    obe.Canvas.InitializeMT(self.elements[id], self.internal:Rectangle(id));
    return self.elements[id];
end

function obe.Canvas.Canvas:Text(id)
    id = self:GenerateId(id);
    --[[self.elements[id] = obe.Canvas.MakeMT({
        obe.Canvas.Bases.Drawable,
        obe.Canvas.Bases.Shape,
        obe.Canvas.Bases.Text}, self.useCache);
    obe.Canvas.InitializeMT(self.elements[id], self.internal:Text(id));]]--
    self.elements[id] = obe.Canvas.InstanciateMT("Text", self.internal:Text(id));
    self.elements[id].font = "Data/Fonts/arial.ttf";
    --print(inspect(self.elements[id]));
    return self.elements[id];
end

function obe.Canvas.Canvas:Circle(id)
    id = self:GenerateId(id);
    self.elements[id] = obe.Canvas.MakeMT({
        obe.Canvas.Bases.Drawable,
        obe.Canvas.Bases.Shape,
        obe.Canvas.Bases.Circle}, self.useCache);
    obe.Canvas.InitializeMT(self.elements[id], self.internal:Circle(id));
    return self.elements[id];
end

function obe.Canvas.Canvas:Sprite(id)
    id = self:GenerateId(id);
    self.elements[id] = { __ref = self.internal:Sprite(id); };
    return self.elements[id];
end

function obe.Canvas.ApplyChangesSub(internal, setters, priority, tbl, cache, force)
    --print("SETTERS", inspect(setters));
    local mt = getmetatable(tbl);
    if #priority > 0 then
        for k, v in pairs(priority) do
            print("Trying to apply", k, v);
            if tbl[v] ~= cache[v] or force then
                obe.Canvas.ApplySetterValue(internal, setters, k, v);
                cache[v] = deepcopy(tbl[v]);
            end
        end
    end
    for k, v in pairs(tbl) do
        if v ~= cache[k] or force then
            --print("Fetching index", k)
            if k ~= "id" then
                obe.Canvas.ApplySetterValue(internal, setters, k, v);
            end
        end
    end
end

function obe.Canvas.ApplyChanges(tbl, force)
    force = force or false;
    local mt = getmetatable(tbl);
    local setters = getmetatable(obe.Canvas.BMT[mt.__type]).__setters;
    local priority = getmetatable(obe.Canvas.BMT[mt.__type]).__priority;
    obe.Canvas.ApplyChangesSub(mt.__ref, setters, priority, tbl, mt.__cache, force);
end

function obe.Canvas.Canvas:render()
    for id, element in pairs(self.elements) do
        local mt = getmetatable(element);
        if #mt.__cache == 0 then
            mt.__cache = deepcopy(element);
            obe.Canvas.ApplyChanges(element, true);
        else
            obe.Canvas.ApplyChanges(element)
            mt.__cache = deepcopy(element);
        end
    end
    --[[if self.useCache then
        for id, element in pairs(self.elements) do
            obe.Canvas.ApplyCache(element);
        end
    end]]--
    self.internal:render();
end

function obe.Canvas.Canvas:setTarget(target)
    self.internal:setTarget(target);
end

function obe.Canvas.Canvas:clear()
    self.elements = {};
    self.internal:clear();
end

function obe.Canvas.Canvas:remove(element)
    self.elements[element] = nil;
    self.internal:remove(element);
end