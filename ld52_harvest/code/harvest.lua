function _init()
    slot = 0
    pulse = 0
    cx = 0
    ctarg_x = 0
    cfruit = 64
    
    -- read the initial grid from the map
    grid={}
    for i=0,4 do
        for j=0,4 do
            local gndx = grid_ndx( i, j)
            
            local dx, dy, ii, jj
            dx = 0
            dy = 0
            ii = i*3+1
            jj = j*3+17
            if mget(ii,jj-1)==44 then dy=-1
            elseif mget(ii,jj+1)==43 then dy=1
            elseif mget(ii-1,jj)==59 then dx=-1
            elseif mget(ii+1,jj)==61 then dx=1            
            end
            
            grid[ grid_ndx(i,j) ] = make_grid( dx, dy )
        end
    end

    
    fruits={}
end

function grid_ndx( col, row )
    return col + row * 5
end

function grid_pos( col, row )
    return 12 + col*24, 16 + row*24
end

function fruit_update( f )
    
    yield()

    local done = 0
    while done==0 do
        -- have we reached our next grid square?
        local tx, ty = grid_pos( f.nxcol, f.nxrow )
        local dx = (f.sx - tx)
        local dy = (f.sy - ty)
        local dd = dx*dx + dy*dy

        if (dd > 5.0) then
            -- not close enough, move closer
            f.sx = lerp( f.sx, tx, 0.2 )
            f.sy = lerp( f.sy, ty, 0.2 )
        else
            -- reached grid cell, choose a new target
            if f.nxrow < 5 then
                
                local gndx = grid_ndx( f.nxcol, f.nxrow )
                local gg = grid[gndx]
                if gg == nil then 
                    done = 1
                else                     
                    f.nxcol = f.nxcol + gg.dx
                    f.nxrow = f.nxrow + gg.dy                    
                end

            else
                done = 1
            end
        end

        yield()
    end
    
    printh("Fruit done.")
end

-- grid cells --
function make_grid( dx, dy )
    return {
        dx=dx,
        dy=dy
    }
end

-- fruits --
function make_fruit( fnum, col, row, sx, sy, upd )
    local ff = {
        fnum = fnum,
        col=col, row=row,
        nxcol=col, nxrow=row,        
        sx=sx, sy=sy,
        tick=cocreate( fruit_update )
    }
    add( fruits, ff )
    assert(coresume( ff.tick, ff ))
end
  
function draw_fruits()    
    for f in all(fruits) do
        spr( f.fnum, f.sx-8, f.sy-8, 2, 2 )
    end
end

function draw_dropfruit()
    spr( cfruit, cx-8, -abs(1.0-pulse)*4  , 2, 2 )
end

-- main stuff --

function lerp(a,b,t)
    return a*(1.0-t) + b*t
end

function _update()

    -- drop cursor
    pulse = sin(time()*2)
    ctarg_x, ctarg_y = grid_pos( slot, 0)
    cx = lerp(cx, ctarg_x, 0.5 )
    if (btnp(0)) then
        slot = slot - 1
        if slot < 0 then slot = 4 end
    end
    
    if (btnp(1)) then
        slot = slot + 1
        if slot > 4 then slot = 0 end
    end

    if (btnp(5)) then
        make_fruit( cfruit,  slot,0, cx, 0)
        cfruit += 2
        if cfruit > 66 then cfruit=64 end
    end
    --printh( "slot" .. slot )

    -- fruit update
    for f in all(fruits) do
        if costatus( f.tick) != "dead" then            
            assert(coresume(f.tick))
        end
    end


end

function _draw()
    
    -- map part
    camera( -4, 0)
    cls()
    map(0,16,0,4)

    -- player sprite
    
    draw_dropfruit()
    draw_fruits()

    -- dbg    
    -- for i=0,4 do
    --     for j=0,4 do
    --         local gx, gy = grid_pos( i, j )
    --         circ( gx, gy, 4, 6 )
    --     end
    -- end    
    
end