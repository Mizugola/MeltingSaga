local canvas;

function Local.Init()
    local screen_size = Engine.Window:getSize();
    canvas = obe.Canvas.Canvas(screen_size.x, screen_size.y);

    canvas:Text "hello" {
        text = "Hello, world!",
        size = 128,
        color = obe.Graphics.Color.White,
        x = 0.5,
        y = 0.5,
        unit = obe.Transform.Units.ViewPercentage,
        align = {
            horizontal = "Center",
            vertical = "Center"
        }
    };
end

function Event.Game.Render()
    canvas:render(Object.Sprite);
end