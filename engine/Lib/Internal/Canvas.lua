local Class = require("Lib/StdLib/Class");
local contains = require("Lib/StdLib/Contains");

obe.Canvas = {};

function deepcopy(orig)
    local orig_type = type(orig)
    local copy
    if orig_type == 'table' then
        copy = {}
        for orig_key, orig_value in next, orig, nil do
            copy[deepcopy(orig_key)] = deepcopy(orig_value)
        end
        setmetatable(copy, deepcopy(getmetatable(orig)))
    else -- number, string, boolean, etc
        copy = orig
    end
    return copy
end

obe.Canvas.Canvas = Class(
                        "Canvas", function(self, width, height, usecache)
        self.internal = obe.Graphics.Canvas.Canvas(math.floor(width), math.floor(height));
        self.elements = {};
        self.useCache = usecache or false;
        self.bases = {
            Line = obe.Canvas
                .MakeMT({obe.Canvas.Bases.Drawable, obe.Canvas.Bases.Line}, self.useCache),
            Rectangle = obe.Canvas.MakeMT(
                {obe.Canvas.Bases.Drawable, obe.Canvas.Bases.Shape, obe.Canvas.Bases.Rectangle},
                self.useCache
            ),
            Text = obe.Canvas.MakeMT(
                {obe.Canvas.Bases.Drawable, obe.Canvas.Bases.Shape, obe.Canvas.Bases.Text},
                self.useCache
            ),
            Circle = obe.Canvas.MakeMT(
                {obe.Canvas.Bases.Drawable, obe.Canvas.Bases.Shape, obe.Canvas.Bases.Circle},
                self.useCache
            ),
            Polygon = obe.Canvas.MakeMT(
                {obe.Canvas.Bases.Drawable, obe.Canvas.Bases.Shape, obe.Canvas.Bases.Polygon},
                self.useCache
            ),
            Bezier = obe.Canvas.MakeMT(
                {obe.Canvas.Bases.Drawable, obe.Canvas.Bases.Bezier}, self.useCache
            )
        };
    end
                    );

function obe.Canvas.NormalizeColor(color, base)
    if type(color) == "table" then
        local ncolor = obe.Graphics.Color();
        base = base or {r = 0, g = 0, b = 0, a = 255};
        ncolor.r = math.floor(color.r or base.r);
        ncolor.g = math.floor(color.g or base.g);
        ncolor.b = math.floor(color.b or base.b);
        ncolor.a = math.floor(color.a or base.a);
        return ncolor;
    elseif type(color) == "number" then
        local dalpha = base.a;
        color = math.floor(color);
        return obe.Graphics.Color(color, color, color, dalpha);
    elseif type(color) == "string" then
        local newColor = obe.Graphics.Color();
        newColor:fromString(color);
        return newColor;
    elseif type(color) == "userdata" then
        return color;
    end
end

function obe.Canvas.ConvertHAlign(align)
    if type(align) == "string" then
        if align == "Left" then
            return obe.Graphics.Canvas.TextHorizontalAlign.Left;
        elseif align == "Center" then
            return obe.Graphics.Canvas.TextHorizontalAlign.Center;
        elseif align == "Right" then
            return obe.Graphics.Canvas.TextHorizontalAlign.Right;
        else
            error(
                "Horizontal Alignment", align,
                "does not exists, use one of those [Left, Center, Right]"
            )
        end
    else
        if align == obe.Graphics.Canvas.TextHorizontalAlign.Left then
            return "Left";
        elseif align == obe.Graphics.Canvas.TextHorizontalAlign.Center then
            return "Center";
        elseif align == obe.Graphics.Canvas.TextHorizontalAlign.Right then
            return "Right";
        end
    end
end

function obe.Canvas.ConvertVAlign(align)
    if type(align) == "string" then
        if align == "Top" then
            return obe.Graphics.Canvas.TextVerticalAlign.Top;
        elseif align == "Center" then
            return obe.Graphics.Canvas.TextVerticalAlign.Center;
        elseif align == "Bottom" then
            return obe.Graphics.Canvas.TextVerticalAlign.Bottom;
        else
            error(
                "Vertical Alignment", align,
                "does not exists, use one of those [Top, Center, Botton]"
            )
        end
    else
        if align == obe.Graphics.Canvas.TextVerticalAlign.Top then
            return "Top";
        elseif align == obe.Graphics.Canvas.TextVerticalAlign.Center then
            return "Center";
        elseif align == obe.Graphics.Canvas.TextVerticalAlign.Bottom then
            return "Bottom";
        end
    end
end

obe.Canvas.Bases = {};

function obe.Canvas.ApplyCache(element)
    local mt = getmetatable(element);
    -- print("Cache :", inspect(mt.__cache));
    if #mt.__priority > 0 then
        for k, v in pairs(mt.__priority) do
            if mt.__cache[v] then
                obe.Canvas.__set(element, v, mt.__cache[v]);
                mt.__cache[v] = nil;
            end
        end
    end
    for k, v in pairs(mt.__cache) do
        obe.Canvas.__set(element, k, v);
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
            return k.__getters[key](tbl.ref);
        elseif type(key) == "number" and not k.__getters[key] and k.__getters.__number then
            return setmetatable(tbl, k.__getters.__number(tbl.ref, key));
        elseif type(k.__getters[key]) == "table" then
            -- print("Found table", inspect(k.__getters[key]));
            return setmetatable(tbl, k.__getters[key]);
        end
    end
end

function obe.Canvas.__set(tbl, key, value)
    local k = getmetatable(tbl);
    if type(key) == "number" and not k.__setters[key] and k.__setters.__number then
        k.__setters.__number(tbl.ref, key, value);
    else
        if k.__setters[key] then
            k.__setters[key](tbl.ref, value);
        else
            error("Can't find obe.Canvas.Canvas attribute : " .. tostring(key));
        end
    end
end

function obe.Canvas.__get(tbl, key)
    local k = getmetatable(tbl);
    if type(k.__getters[key]) == "function" then
        return k.__getters[key](tbl.ref);
    elseif type(key) == "number" and not k.__getters[key] and k.__getters.__number then
        k.__getters.__number(tbl.ref)
    elseif type(k.__getters[key]) == "table" then
        if rawget(tbl, key) == nil then
            rawset(tbl, key, setmetatable({ref = rawget(tbl, "ref")}, k.__getters[key]));
        end
        return rawget(tbl, key);
    end
end

function obe.Canvas.__call(tbl, values, usecache)
    values = values or {};
    local mt = getmetatable(tbl);
    if usecache then
        for k, v in pairs(values) do
            mt.__cache[k] = v;
        end
    else
        if #mt.__priority > 0 then
            for k, v in pairs(mt.__priority) do
                if values[v] then
                    obe.Canvas.__set(tbl, v, values[v]);
                    values[v] = nil;
                end
            end
        end
        for k, v in pairs(values) do
            obe.Canvas.__set(tbl, k, v);
        end
    end
    return tbl;
end

function obe.Canvas.MakeMT(bases, usecache)
    usecache = usecache or false;
    local getters = {};
    local setters = {};
    local priority = {};

    for _, base in pairs(bases) do
        if base.priority then
            for _, priorityName in pairs(base.priority) do
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
        __getters = getters,
        __setters = setters,
        __priority = priority,
        __index = getfunc,
        __newindex = setfunc,
        __call = function(t, v)
            return obe.Canvas.__call(t, v, usecache)
        end,
        __cache = {},
        __rcache = {}
    };

    return mt;
end

function obe.Canvas.Canvas:InstanciateMT(type, internal)
    local mt = self.bases[type];
    return setmetatable({type = type, ref = internal}, mt);
end

obe.Canvas.Bases.Drawable = {
    getters = {
        layer = function(self)
            return self.layer;
        end,
        visible = function(self)
            return self.visible;
        end,
        id = function(self)
            return self:getId();
        end
    },
    setters = {
        layer = function(self, layer)
            self:setLayer(layer or 1);
        end,
        visible = function(self, visible)
            self.visible = visible;
        end
    }
}

obe.Canvas.Bases.Line = {
    getters = {
        p1 = {
            priority = {"unit"},
            getters = {
                x = function(self)
                    return self.p1.x;
                end,
                y = function(self)
                    return self.p1.y;
                end,
                unit = function(self)
                    return self.p1.unit;
                end,
                color = {
                    getters = {
                        r = function(self)
                            return self.p1color.r;
                        end,
                        g = function(self)
                            return self.p1color.g;
                        end,
                        b = function(self)
                            return self.p1color.b;
                        end,
                        a = function(self)
                            return self.p1color.a;
                        end
                    },
                    setters = {
                        r = function(self, r)
                            self.p1color.r = r or 0;
                        end,
                        g = function(self, g)
                            self.p1color.g = g or 0;
                        end,
                        b = function(self, b)
                            self.p1color.b = b or 0;
                        end,
                        a = function(self, a)
                            self.p1color.a = a or 255;
                        end
                    }
                }
            },
            setters = {
                x = function(self, x)
                    self.p1.x = x or 0;
                end,
                y = function(self, y)
                    self.p1.y = y or 0;
                end,
                unit = function(self, unit)
                    self.p1.unit = unit or obe.Transform.Units.ScenePixels;
                end,
                color = function(self, color)
                    self.p1color = obe.Canvas.NormalizeColor(color, self.p1color);
                end
            }
        },
        p2 = {
            priority = {"unit"},
            getters = {
                x = function(self)
                    return self.p2.x;
                end,
                y = function(self)
                    return self.p2.y;
                end,
                unit = function(self)
                    return self.p2.unit;
                end,
                color = {
                    getters = {
                        r = function(self)
                            return self.p2color.r;
                        end,
                        g = function(self)
                            return self.p2color.g;
                        end,
                        b = function(self)
                            return self.p2color.b;
                        end,
                        a = function(self)
                            return self.p2color.a;
                        end
                    },
                    setters = {
                        r = function(self, r)
                            self.p2color.r = r or 0;
                        end,
                        g = function(self, g)
                            self.p2color.g = g or 0;
                        end,
                        b = function(self, b)
                            self.p2color.b = b or 0;
                        end,
                        a = function(self, a)
                            self.p2color.a = a or 255;
                        end
                    }
                }
            },
            setters = {
                x = function(self, x)
                    self.p2.x = x or 0;
                end,
                y = function(self, y)
                    self.p2.y = y or 0;
                end,
                unit = function(self, unit)
                    self.p2.unit = unit or obe.Transform.Units.ScenePixels;
                end,
                color = function(self, color)
                    self.p2color = obe.Canvas.NormalizeColor(color, self.p2color);
                end
            }
        },
        unit = function(self)
            return self.p1.unit;
        end,
        thickness = function(self)
            return self.thickness;
        end,
        color = {
            getters = {
                r = function(self)
                    return self.p1color.r;
                end,
                g = function(self)
                    return self.p1color.g;
                end,
                b = function(self)
                    return self.p1color.b;
                end,
                a = function(self)
                    return self.p1color.a;
                end
            },
            setters = {
                r = function(self, r)
                    self.p1color.r = r or 0;
                    self.p2color.r = r or 0;
                end,
                g = function(self, g)
                    self.p1color.g = g or 0;
                    self.p2color.g = g or 0;
                end,
                b = function(self, b)
                    self.p1color.b = b or 0;
                    self.p2color.b = b or 0;
                end,
                a = function(self, a)
                    self.p1color.a = a or 255;
                    self.p2color.a = a or 255;
                end
            }
        }
    },
    setters = {
        unit = function(self, unit)
            self.p1.unit = unit or obe.Transform.Units.ScenePixels;
            self.p2.unit = unit or obe.Transform.Units.ScenePixels;
        end,
        thickness = function(self, thickness)
            self.thickness = thickness or 1;
        end,
        color = function(self, color)
            self.p1color = obe.Canvas.NormalizeColor(color, self.p1color);
            self.p2color = obe.Canvas.NormalizeColor(color, self.p2color);
        end,
        p1 = function(self, p1)
            if type(p1) == "table" then
                if type(p1.x) == "number" then
                    self.p1.x = p1.x;
                end
                if type(p1.y) == "number" then
                    self.p1.y = p1.y;
                end
                if p1.color then
                    self.p1color = obe.Canvas.NormalizeColor(p1.color, self.p1color);
                end
                if p1.unit then
                    self.p1.unit = p1.unit or obe.Transform.Units.ScenePixels;
                end
            end
        end,
        p2 = function(self, p2)
            if type(p2) == "table" then
                if type(p2.x) == "number" then
                    self.p2.x = p2.x;
                end
                if type(p2.y) == "number" then
                    self.p2.y = p2.y;
                end
                if p2.color then
                    self.p2color = obe.Canvas.NormalizeColor(p2.color, self.p2color);
                end
                if p2.unit then
                    self.p2.unit = p2.unit or obe.Transform.Units.ScenePixels;
                end
            end
        end
    }
};

obe.Canvas.Bases.Shape = {
    priority = {"unit"},
    getters = {
        x = function(self)
            return self.position.x;
        end,
        y = function(self)
            return self.position.y;
        end,
        unit = function(self)
            return self.position.unit;
        end,
        angle = function(self)
            return self.shape:getRotation();
        end,
        scale = {
            getters = {
                x = function(self)
                    return self.shape:getScale().x;
                end,
                y = function(self)
                    return self.shape:getScale().y;
                end
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
                        r = function(self)
                            return self.shape:getOutlineColor().r;
                        end,
                        g = function(self)
                            return self.shape:getOutlineColor().g;
                        end,
                        b = function(self)
                            return self.shape:getOutlineColor().b;
                        end,
                        a = function(self)
                            return self.shape:getOutlineColor().a;
                        end
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
                thickness = function(self)
                    return self.shape:getOutlineThickness();
                end
            },
            setters = {
                color = function(self, color)
                    self.shape:setOutlineColor(
                        obe.Canvas.NormalizeColor(
                            color, self.shape:getOutlineColor()
                        )
                    );
                end,
                thickness = function(self, thickness)
                    self.shape:setOutlineThickness(thickness or 1);
                end
            }
        },
        color = {
            getters = {
                r = function(self)
                    return self.shape:getFillColor().r;
                end,
                g = function(self)
                    return self.shape:getFillColor().g;
                end,
                b = function(self)
                    return self.shape:getFillColor().b;
                end,
                a = function(self)
                    return self.shape:getFillColor().a;
                end
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
                    self.shape:setOutlineColor(
                        obe.Canvas.NormalizeColor(
                            outline.color, self.shape:getOutlineColor()
                        )
                    );
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
            self.shape:setPosition(self.position);
        end,
        y = function(self, y)
            self.position.y = y;
            self.shape:setPosition(self.position);
        end,
        unit = function(self, unit)
            self.position.unit = unit or obe.Transform.Units.ScenePixels;
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
        end,
        texture = function(self, texture)
            local texture_path = obe.System.Path(texture);
            self.shape:setTexture(Engine.Resources:getTexture(texture_path));
        end
    }
}

obe.Canvas.Bases.Rectangle = {
    priority = {"unit"},
    getters = {
        width = function(self)
            return self.size.x;
        end,
        height = function(self)
            return self.size.y;
        end
    },
    setters = {
        width = function(self, width)
            self.size.x = width;
            self.shape:setSize(self.size);
        end,
        height = function(self, height)
            self.size.y = height
            self.shape:setSize(self.size);
        end,
        unit = function(self, unit)
            self.position.unit = unit or obe.Transform.Units.ScenePixels;
            self.size.unit = unit or obe.Transform.Units.ScenePixels;
        end
    }
}

obe.Canvas.Bases.Circle = {
    getters = {
        radius = function(self)
            return self.shape:getRadius();
        end,
        width = function(self)
            return self.shape:getGlobalBounds().width;
        end,
        height = function(self)
            return self.shape:getGlobalBounds().height;
        end
    },
    setters = {
        radius = function(self, radius)
            return self.shape:setRadius(radius);
        end
    }
}

function positionToUnitVector(position)
    if type(position) == "table" then
        local x = position.x or position[1];
        local y = position.y or position[2];
        local unit = position.unit or position[3] or obe.Transform.Units.ScenePixels;
        position = obe.Transform.UnitVector(x, y, unit);
    end
    return position;
end
obe.Canvas.Bases.Polygon = {
    getters = {
        points = {
            getters = {
                __number = function(self, index)
                    return self.shape:getPointPosition(index);
                end
            },
            setters = {
                __number = function(self, index, position)
                    position = positionToUnitVector(position);
                    self.shape:setPointPosition(index - 1, position);
                end
            }
        }
    },
    setters = {
        points = function(self, points)
            for k, v in pairs(points) do
                local position = positionToUnitVector(v);
                self.shape:setPointPosition(k - 1, position);
            end
        end
    }
}

obe.Canvas.Bases.Bezier = {
    getters = {
        points = {
            getters = {
                __number = function(self, index)
                    return self.points[index];
                end
            },
            setters = {
                __number = function(self, index, position)
                    self.points[index - 1] = positionToUnitVector(position);
                end
            }
        },
        precision = function(self)
            return self.precision;
        end
    },
    setters = {
        points = function(self, points)
            for k, v in pairs(points) do
                local color = v.color or self.colors[k - 1] or obe.Graphics.Color(255, 255, 255);
                color = obe.Canvas.NormalizeColor(color);
                table.insert(self.points, positionToUnitVector(v));
                table.insert(self.colors, color);
                -- self.points[k - 1] = positionToUnitVector(v);
            end
        end,
        precision = function(self, precision)
            self.precision = precision;
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
    priority = {"font", "size", "color", "outline", "align"},
    getters = {
        text = function(self)
            local fullText = "";
            for _, text in pairs(self.texts) do
                fullText = fullText .. text.string;
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
                h = function(self)
                    return obe.Canvas.ConvertHAlign(self.h_align);
                end,
                horizontal = function(self)
                    return obe.Canvas.ConvertHAlign(self.h_align);
                end,
                v = function(self)
                    return obe.Canvas.ConvertVAlign(self.v_align);
                end,
                vertical = function(self)
                    return obe.Canvas.ConvertVAlign(self.v_align);
                end
            },
            setters = {
                h = function(self, h)
                    self.h_align = obe.Canvas.ConvertHAlign(h);
                end,
                horizontal = function(self, h)
                    self.h_align = obe.Canvas.ConvertHAlign(h);
                end,
                v = function(self, v)
                    self.v_align = obe.Canvas.ConvertVAlign(v);
                end,
                vertical = function(self, v)
                    self.v_align = obe.Canvas.ConvertVAlign(v);
                end
            }
        },
        color = {
            getters = {
                r = function(self)
                    return self.text.color.r;
                end,
                g = function(self)
                    return self.text.color.g;
                end,
                b = function(self)
                    return self.text.color.b;
                end,
                a = function(self)
                    return self.text.color.a;
                end
            },
            setters = {
                r = function(self, r)
                    self.text.color.r = r;
                    self:refresh();
                end,
                g = function(self, g)
                    self.text.color.g = g;
                    self:refresh();
                end,
                b = function(self, b)
                    self.text.color.b = b;
                    self:refresh();
                end,
                a = function(self, a)
                    self.text.color.a = a;
                    self:refresh();
                end
            }
        }
    },
    setters = {
        text = function(self, text)
            if self.fontPath == "" then
                error("@Canvas.Text.setters.text : Need to set @font before @text");
            end
            self.text.string = text;
            self:refresh();
        end,
        size = function(self, size)
            self.shape:setCharacterSize(size);
        end,
        font = function(self, font)
            self.fontPath = font;
            self.shape:setFont(Engine.Resources:getFont(font));
        end,
        color = function(self, color)
            self.text.color = obe.Canvas.NormalizeColor(color);
            self:refresh();
        end,
        outline = function(self, outline)
            if type(outline) == "table" then
                if outline.color then
                    self.text.outline = obe.Canvas.NormalizeColor(outline.color);
                    self:refresh();
                end
                if type(outline.thickness) == "number" then
                    self.text.thickness = outline.thickness;
                    self:refresh();
                end
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

function obe.Canvas.Canvas:GenerateId(id)
    if type(id) == "string" and self.internal:get(id) ~= nil then
        error("CanvasElement '" .. tostring(id) .. "' already exists !");
    end
    while id == nil or self.internal:get(id) ~= nil do
        id = obe.Utils.String
                 .getRandomKey(obe.Utils.String.Alphabet .. obe.Utils.String.Numbers, 12);
    end
    return id;
end

function obe.Canvas.Canvas:Line(id)
    id = self:GenerateId(id);
    self.elements[id] = self:InstanciateMT("Line", self.internal:Line(id));
    -- print(inspect(self.elements[id]));
    return self.elements[id];
end

function obe.Canvas.Canvas:Rectangle(id)
    id = self:GenerateId(id);
    self.elements[id] = self:InstanciateMT("Rectangle", self.internal:Rectangle(id));
    return self.elements[id];
end

function obe.Canvas.Canvas:Text(id)
    id = self:GenerateId(id);
    self.elements[id] = self:InstanciateMT("Text", self.internal:Text(id));
    self.elements[id].font = "Data/Fonts/NotoSans.ttf";
    return self.elements[id];
end

function obe.Canvas.Canvas:Circle(id)
    id = self:GenerateId(id);
    self.elements[id] = self:InstanciateMT("Circle", self.internal:Circle(id));
    return self.elements[id];
end

function obe.Canvas.Canvas:Polygon(id)
    id = self:GenerateId(id);
    self.elements[id] = self:InstanciateMT("Polygon", self.internal:Polygon(id));
    return self.elements[id];
end

function obe.Canvas.Canvas:Bezier(id)
    id = self:GenerateId(id);
    self.elements[id] = self:InstanciateMT("Bezier", self.internal:Bezier(id));
    return self.elements[id];
end

function obe.Canvas.Canvas:Sprite(id)
    id = self:GenerateId(id);
    self.elements[id] = {ref = self.internal:Sprite(id)};
    return self.elements[id];
end

function obe.Canvas.Canvas:render(target)
    if self.useCache then
        for id, element in pairs(self.elements) do
            obe.Canvas.ApplyCache(element);
        end
    end
    self.internal:render(target);
end

function obe.Canvas.Canvas:clear()
    self.elements = {};
    self.internal:clear();
end

function obe.Canvas.Canvas:remove(element)
    self.elements[element] = nil;
    self.internal:remove(element);
end
