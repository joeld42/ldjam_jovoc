function _init()
    slot = 0
    pulse = 0
    cx = 0
    ctarg_x = 0
    cfruit = 64
    nxtfruit = 66
    vp = 0
    fruitleft = 0
    
    shopy = 129
    shopytarg = 70
    shopsel=0
    shopdone=true
    buildx=-1
    buildy=-1
    bgx=0
    bgy=0

    msgs = {}
    
    shopitems = {}    
    shopitemdirs = {
        { mapx=17, dx=0, dy=1 },
        { mapx=21, dx=-1, dy=0 },
        { mapx=25, dx=1, dy=0 },
        { mapx=29, dx=0, dy=-1 },
    }

    shopitembuilds = {
        { name="pOLISHER", price=5, icon=192 },
        { name="bLENDER", price=8, icon=194 },
        { name="cLONEjAR", price=10, icon=194 },
        { name="cANNER", price=6, icon=194 },
    }

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
                    -- move to the next grid                    
                    f.nxcol = f.nxcol + gg.dx
                    f.nxrow = f.nxrow + gg.dy                    

                    -- age the fruit
                    f.age = f.age + 1
                    if (f.age > 10) then
                        -- todo spawn spoilage particles
                        spawn_msg( f.sx-12, f.sy, "Spoiled!", 11 )
                        return
                    end
                end

            else
                -- Convert fruit to score
                score_fruit(f)
                done = 1
            end
        end

        yield()
    end
    
end

function score_fruit( f )
    local p = flr(rnd(3))+1
    vp = vp + p
    spawn_msg( f.sx+(rnd(12)-10), f.sy-10, tostr(p), 10 )
end


function spawn_msg( x,y, msg, c )
    add( msgs, {
        x=x, y=y, msg=msg, c=c,
        age=0
    })
end


-- grid cells --
function make_grid( dx, dy )
    return {
        dx=dx,
        dy=dy,
        icon=0
    }
end

-- fruits --
function make_fruit( fnum, col, row, sx, sy, upd )
    local ff = {
        fnum = fnum,
        col=col, row=row,
        nxcol=col, nxrow=row,        
        sx=sx, sy=sy,
        age=0,
        vp=1,
        tick=cocreate( fruit_update )
    }
    add( fruits, ff )
    assert(coresume( ff.tick, ff ))
end
  
function draw_fruits()    
    
    local spoil_age = 6
    for f in all(fruits) do
        if f.age < spoil_age then                         
            spr( f.fnum, f.sx-8, f.sy-8, 2, 2 )
        end
    end

    -- draw fruits about to spoil with a different palette
    pal( { [0]=3,3,3,3,  3,3,11,11,  3,3,11,7,  11,11,11,11 })
    for f in all(fruits) do
        if f.age >= spoil_age then                         
            spr( f.fnum, f.sx-8, f.sy-8, 2, 2 )
        end
    end

    pal()

end

function draw_dropfruit()
    spr( cfruit, cx-8, -abs(1.0-pulse)*4  , 2, 2 )
end

function draw_msgs()
    local active_msgs = {}
    for m in all(msgs) do
        if (m.age < 30) or (m.age & 2 ==0) then
            print( m.msg, m.x-1, m.y, 0 )
            print( m.msg, m.x, m.y-1, 0 )
            print( m.msg, m.x+1, m.y, 0 )
            print( m.msg, m.x, m.y+1, 0 )

            print( m.msg, m.x, m.y, m.c )
        end
        m.age += 1
        m.y -= 1
        if m.age < 45 then
            add( active_msgs, m )
        end
    end
    msgs = active_msgs
end

-- main stuff --

function lerp(a,b,t)
    return a*(1.0-t) + b*t
end

function fruit_cursor()
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

        fruitleft -= 1

        cfruit = nxtfruit
        nxtfruit += 2
        if nxtfruit > 66 then nxtfruit=64 end
    end
    
end

function open_shop()


    while #shopitems < 3 do

        genitem = {}

        local genitemdir = rnd( shopitemdirs )
        local genitembuild = rnd( shopitembuilds )
        
        for k,v in pairs(genitemdir) do            
            genitem[k] = v
        end
        for k,v in pairs(genitembuild) do
            genitem[k] = v
        end

        for k,v in pairs(genitem) do
            printh( tostr(k) .. " -> " .. tostr(v) )
        end

        add( shopitems, genitem )
    end

    shopytarg = 70
    shopdone = false
end

function shop_update()
    

    -- choose item menu
    if (buildx < 0)then
        if (btnp(0)) then
            shopsel -= 1
            if shopsel < 0 then shopsel = 3 end
        elseif (btnp(1)) then
            shopsel += 1
            if shopsel > 3 then shopsel = 0 end
        end

        if (btnp(5)) then
            shopytarg = 129        
            if shopsel==3 then 
                -- done button pressed
                shopdone = true
            else
                -- buy/build an item
                buildx = 3
                buildy = 3
                bgx = 64
                bgy=128
    
            end
        end    

    else
        -- place building
        if (btnp(0)) then 
            buildx -= 1
            if (buildx < 0) then buildx = 4 end
        end
        if (btnp(1)) then 
            buildx += 1
            if (buildx > 4) then buildx = 0 end
        end
        if (btnp(2)) then 
            buildy -= 1
            if (buildy < 0) then buildy = 4 end
        end
        if (btnp(3)) then 
            buildy += 1
            if (buildy > 4) then buildy = 0 end
        end

        if (btnp(5)) then 
            -- build
            printh("BUILD")
            local bitem = shopitems[ shopsel+1]
            local gg = make_grid( bitem.dx, bitem.dy )            
            gg.icon = 192
            
            grid[ grid_ndx(buildx,buildy) ] = gg
            
            -- copy build tile to map
            for i=0,2 do
                for j=0,2 do
                    local gx, gy = grid_pos( i, j )
                    local mt = mget( bitem.mapx + i, 16+j )
                    mset( buildx*3 + i, 16+buildy*3+j, mt )
                end
            end    

        elseif (btnp(4)) then
            buildx = -1
            buildy = -1
            shopytarg = 70
        end
          
    end

    
end

function _update()

    -- drop cursor
    if fruitleft > 0 then fruit_cursor() 
    elseif #fruits == 0 then
        if (#shopitems == 0) then 
            open_shop()
        end
        shop_update()    
    end
    
    -- fruit update
    local active_fruits = {}
    for f in all(fruits) do
        if costatus( f.tick) != "dead" then            
            assert(coresume(f.tick))
            add( active_fruits, f )
        end
    end
    fruits = active_fruits    
end

function shop_draw()

    shopy = lerp( shopy, shopytarg, 0.2 ) 

    -- moar fruit, back to game??
    if (shopdone and shopy >= 127) then
        fruitleft = 20
        shopitems = {}
    end

    -- building?
    if (buildx >= 0) then
        pal( 5, 13 )
        pal( 13, 12 )
        local gx, gy = grid_pos( buildx, buildy )
        bgx = lerp( bgx, gx, 0.3 )
        bgy = lerp( bgy, gy, 0.3 )
        rect( bgx-14, bgy-14, bgx+14, bgy+14, 12 )

        bitem = shopitems[ shopsel+1 ]
        map( bitem.mapx, 16, bgx-12, bgy-12, 4, 4 )
        spr( bitem.icon, bgx-8, bgy-8, 2, 2 )

        --spr( gg.icon, gx-8, gy-8, 2, 2 )
        
        local s = "❎ build 🅾️ cancel" 
        print( s, 26, 121, 0)
        print( s, 25, 120, 7)
        
        pal( 5, 5 )
        pal( 13, 13 )
    end    

    -- draw buy panel
    camera(0,0)
    rectfill( 0, shopy, 34, shopy+10, 13 )
    rectfill( 0, shopy+10, 128, 128, 13 )
    print( "upgrade", 4, shopy+2, 12 )
    for ndx, item in ipairs(shopitems) do
        local xx = 8 + (ndx-1) * 35

        if (shopsel == ndx-1) then
            rectfill( xx-4, shopy+11, xx+28, shopy+55, 12 )
        end

        print( item.name, xx-3, shopy+12, 7)
        map( item.mapx, 16, xx, shopy+20, 4, 4 )
        spr( item.icon, xx+4, shopy+23, 2, 2 )

        spr( 16, xx+2, shopy+46 )
        local cc = tostr(item.price)
        print( cc, xx+13, shopy+48, 0 )
        print( cc, xx+12, shopy+47, 7 )
    end

    -- Done button        
    rectfill( 106, shopy+30, 124, shopy+38, (shopsel==3 and 12 or 6 ) )
    print( "done", 108, shopy+32, (shopsel==3 and 7 or 5 ) )
end

function _draw()
    
    -- map part
    camera( -3, 0)
    cls(3)
    map(0,16,0,4)

    for i=0,4 do
        for j=0,4 do
            local gx, gy = grid_pos( i, j )
            local gg = grid[ grid_ndx( 0, j )]
            if (gg.icon != 0) then
                spr( gg.icon, gx-8, gy-8, 2, 2 )
            end
            --circ( gx, gy, 4, 6 )
        end
    end

    -- player sprite
    
    if (fruitleft > 0 ) then 
        draw_dropfruit()
    end
    draw_fruits()
    draw_msgs()

    -- hud VP (coins)
    spr( 16, 100, 2 )
    local cc = tostr(vp)
    print( cc, 111, 4, 0 )
    print( cc, 110, 3, 7 )

    -- fruit left
    if fruitleft > 0 then
        local sx, sy = (nxtfruit % 16) * 8, flr(nxtfruit \ 16) * 8
        --spr( nxtfruit, 100, 2 )
        sspr( sx, sy, 16, 16, 100, 12, 8, 8 )
        local fl = tostr(fruitleft)
        print( fl, 111, 14, 0 )
        print( fl, 110, 13, 7 )
    end

    -- buy round    
    if #shopitems > 0 then   
        shop_draw()
    end

    -- dbg    
    -- for i=0,4 do
    --     for j=0,4 do
    --         local gx, gy = grid_pos( i, j )
    --         circ( gx, gy, 4, 6 )
    --     end
    -- end    
    
end