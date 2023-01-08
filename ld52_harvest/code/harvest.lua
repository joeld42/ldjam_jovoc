

function _init()
    slot = 0
    mode = 1 -- 0 = title, 1 = harvest, 2 = game, 3 = scoreboard
    pulse = 0
    cx = 0
    ctarg_x = 0
    
    cfruit = 1
    nxtfruit = cfruit+1
    vp = 0
    fruitleft = 5
    
    gmy = 0
    gmytarg = 16*8+4

    shopy = 129
    shopytarg = 70
    shopsel=0
    shopdone=true
    buildx=-1
    buildy=-1
    bgx=0
    bgy=0
    buildleft=0

    round = 1

    msgs = {}

    roundinfo = {
        { name="fARMERS mARKET", desc="WARM UP!\nNO SPECIAL RULES", fkind=99, fmul=1 },
        
        { name="aPPLE a dAY", desc="APPLE ITEMS X3", fkind=1, fmul=3 },
        { name="oRANGE yOU gLAD", desc="CITRUS ITEMS X2", fkind=2, fmul=2 },        
        { name="pUMPKIN sPICE sEASON", desc="PUMPKIN ITEMS X2", fkind=3, fmul=2  },
        { name="sTRAWBERRY fESTIVAL", desc="STRAWBERRY ITEMS X2", fkind=4, fmul=2  },
        { name="aQUAS fRESCAS", desc="WATERMELON ITEMS X2", fkind=5, fmul=2  },                
    }
    rinfo = roundinfo[1]
    
    buyerinfo = { 
        { name="jUICE bAR", desc="SMOOTHIES\nSCORE DOUBLE" },
        { name="bAKERY", desc="BAKED GOODS\nSCORE DOUBLE"  },
        { name="fARMERS mARKET", desc="NORMAL SCORING" },
        { name="fANCY gROCER", desc="ORGANIC FOODS\nSCORE DOUBLE" },
        { name="wHOLESALE cLUB", desc="PRESERVED FOODS\nSCORE DOUBLE" },
    }
    fruitinfo = {
        { name="apple", fkind=1, fnum=64, cbase=8, cdark=2, cbrite=14, vp = 1 },
        { name="orange", fkind=2, fnum=64, cbase=9, cdark=4, cbrite=10, vp = 2 },
        { name="pumpkin", fkind=3, fnum=66, cbase=4, cdark=2, cbrite=9, vp = 3 },
        { name="strawberry", fkind=4,fnum=66, cbase=8, cdark=3, cbrite=14, vp = 4 },
        { name="watermelon", fkind=5, fnum=68, cbase=8, cdark=2, cbrite=14, vp = 5 },
    }
    
    shopitems = {}    
    shopitemdirs = {
        { mapx=17, dx=0, dy=1 },
        { mapx=21, dx=-1, dy=0 },
        { mapx=25, dx=1, dy=0 },
        -- { mapx=29, dx=0, dy=-1 },
    }

    shopitembuilds = {
        { name="fANCIFIER", icon=194 },
        { name="bLENDER",  icon=196 },
        { name="cLONEjAR", icon=198 },
        { name="cANNER",  icon=192 },
        { name="pIEINATOR", icon=204 },
        { name="mANURE",  icon=202 },
        { name="sORTER",  icon=200 },
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

-- Save copied tables in `copies`, indexed by original table.
-- from the internets
function deepcopy(orig, copies)
    copies = copies or {}
    local orig_type = type(orig)
    local copy
    if orig_type == 'table' then
        if copies[orig] then
            copy = copies[orig]
        else
            copy = {}
            copies[orig] = copy
            for orig_key, orig_value in next, orig, nil do
                copy[deepcopy(orig_key, copies)] = deepcopy(orig_value, copies)
            end
            setmetatable(copy, deepcopy(getmetatable(orig), copies))
        end
    else -- number, string, boolean, etc
        copy = orig
    end
    return copy
end

function grid_ndx( col, row )
    if (col < 0) or (col >= 5) or
        (row < 0) or (row >= 5) then
            return nil
        end
    return col + row * 5
end

function grid_pos( col, row )
    return 12 + col*24, (16*9) + row*24
end

function fruit_update( f )
    
    yield()

    while f.delay > 0 do
        f.delay -= 1
        yield()
    end

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
                    local dx = gg.dx
                    local dy = gg.dy
                    if (gg.icon == 200) then
                        dx, dy = choose_dir( f )
                    end
                    f.nxcol = f.nxcol + dx
                    f.nxrow = f.nxrow + dy

                    -- check if the fruit has visited this spot before
                    if f.visited[gndx] or (f.nxcol < 0) or (f.nxcol > 4) then
                        spawn_msg( f.sx-12, f.sy, "Malfunction!", 11 )
                        return
                    end
                    f.visited[gndx] = true
                    
                    
                    -- process the fruit
                    process_fruit( f, gg, gndx )
                    
                end

            else
                -- Convert fruit to score
                local vpfruit = score_fruit(f, f.nxcol )       
                
                printh( "SCORED " .. tostr(vpfruit) )

                vp = vp + vpfruit
                name = describe_fruit(f) .. " $" .. tostr(vpfruit)
                spawn_msg( f.sx-20, f.sy-20, name, 10 )

                done = 1
            end
        end

        yield()
    end
    
end

function describe_fruit( f )
    local name = f.name
    if f.canned then 
        name = "canned\n" .. name
    end

    if f.organic then 
        name = "organic\n" .. name
    end

    if f.fancy > 0 then
        local fnames = { "fancy", "extrafancy", "deluxe", "heirloom", "boutique", "artisanal", "bespoke" }
        local n = f.fancy
        if n > #fnames then n = #fnames end
        name = fnames[n] .. "\n" .. name
    end

    if f.smoothie and f.baked then 
        name = name .. "\nsilk pie"
    elseif f.baked then
        name = name .. "\npie"
    elseif f.smoothie then
        name = name .. "\nsmoothie"
    end

    return name
end

function predict_score( f )
    
    local x = f.nxcol
    local y = f.nxrow
    if (y > 4) then 
        return score_fruit( f, x )
    end
    
    local gndx = grid_ndx( x, y)         

    if f.visited[gndx] then
        return 0
    end

    if (x < 0) or (x > 4) then
        -- malfunction
        return 0
    end

    local gg = grid[ gndx ]
    local mul = 1
    if gg.icon == 198 then
        -- don't modify fruit, just double the result
        mul = 2
    else
        process_fruit( f, gg, gndx )
    end

    f.visited[gndx] = true

    -- move to the next grid 
    local dx = gg.dx
    local dy = gg.dy
    if (gg.icon == 200) then
        dx, dy = choose_dir( f ) -- recurse
    end    
    f.nxcol = x + dx
    f.nxrow = y + dy    
    return mul * predict_score( f )

    
end

function choose_dir( f )
    -- for now random sort
    local dx1, dy1
    local best_score = 0

    for dd=0,2 do
        local dx, dy
        if dd==0 then 
            dx = -1
            dy = 0
        elseif dd==1 then 
            dx = 0
            dy = 1
        else
            dx = 1
            dy = 0
        end
        
        local ff = deepcopy( f )
        ff.tick = nil

        ff.nxcol = f.nxcol + dx
        ff.nxrow = f.nxrow + dy
        ff.visited[ grid_ndx( f.nxcol, f.nxrow)] = true

        -- add a small random tiebreak
        local fs = predict_score( ff ) + rnd(0.01)
        printh( "dir " .. tostr(dx) .. " " .. tostr(dy) .. " score " .. tostr(fs) )
        if fs > best_score then
            dx1 = dx
            dy1 = dy
            best_score = fs
        end
    end

    printh("best score " ..flr(best_score) .. " dxy " .. tostr(dx1) .. "  " ..tostr(dy1) )

    return dx1, dy1
end

function process_fruit( f, g, gndx )
    if (g.icon == 192) then
        -- canner
        f.fnum = 96
        f.canned = true
        f.baked = false
    elseif (g.icon == 202) then
        -- manure pile
        f.coutline = 11
        f.organic = true        
    elseif (g.icon == 198) then
        -- clone jar
        clone_fruit(f,gndx)
    elseif (g.icon == 196) then
        -- blender
        f.fnum = 98
        f.smoothie = true
        f.baked = false
    elseif (g.icon == 204) then        
        -- pieinator
        f.baked = true
        f.fnum = 100
    elseif (g.icon == 194) then
        -- fancifier        
        f.vp = f.vp+1
        f.fancy += 1
    end
end

function score_fruit( f, buyer )
    
    -- start with base value
    local vp = f.vp

    -- do modifiers
    if buyer==0 then
        if f.smoothie then
            vp *= 2 -- juice bar
        end
    elseif buyer==1 then
        if f.baked then
            vp *= 2 -- bakery
        end
    elseif buyer==4 then
        if f.organic then
            vp *= 2 -- fancy grocery
        end
    elseif buyer==5 then
        if f.canned then
            vp *= 2 -- wholesaler
        end
    end

    -- round modifiers
    printh ("checking " .. tostr(rinfo.fkind) .. " " .. tostr(f.fkind) )    
    if f.fkind == rinfo.fkind then
        vp *= rinfo.fmul
        printh ("x " .. (rinfo.fmul) .. " vp " .. tostr(vp) )            
    end
    
    return vp    
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
function make_fruit( finfo_ndx, col, row, sx, sy, upd )
    local finfo = fruitinfo[finfo_ndx]
    local ff = {
        fnum = finfo.fnum,
        fkind = finfo.fkind,
        name= finfo.name,
        cbase = finfo.cbase,
        cdark = finfo.cdark,
        cbrite = finfo.cbrite,
        coutline = 7,
        col=col, row=row,
        nxcol=col, nxrow=row,        
        sx=sx, sy=sy,
        visited={},
        vp=finfo.vp,  
        delay=0,

        -- flags
        organic=false,
        smoothie=false,
        canned=false,
        fancy=0,

        tick=cocreate( fruit_update )
    }
    add( fruits, ff )
    assert(coresume( ff.tick, ff ))
end

function clone_fruit( f, gndx )
    local ff = {
        tick=cocreate( fruit_update )
    }
    for k,v in pairs(f) do
        if k!="tick" then
            ff[k] = v
        end
    end
    ff.visited = {}
    ff.visited[gndx] = true
    ff.delay=20

    add( fruits, ff )
    assert(coresume( ff.tick, ff ))
end

function draw_fruits()    
    
    for f in all(fruits) do
        pal( 8, f.cbase ) -- base color
        pal( 2, f.cdark ) -- dark color
        pal( 14, f.cbrite ) -- highlight color
        pal( 7, f.coutline ) -- outline color
        spr( f.fnum, f.sx-8, f.sy-8, 2, 2 )
    end

    -- restore pallette
    pal()

end

function draw_dropfruit()
    local fi = fruitinfo[ cfruit ]
    pal( 8, fi.cbase ) -- base color
    pal( 2, fi.cdark ) -- dark color
    pal( 14, fi.cbrite ) -- highlight color        
    spr( fi.fnum, cx-8, (16*8) -abs(1.0-pulse)*4  , 2, 2 )
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
    
    if (btnp(3)) then
        mode = 2
        gmytarg = 30*8
    end

    if (btnp(2)) then
        mode = 1
        gmytarg = 16*8+2
    end

    

    if (btnp(5)) then
        
        make_fruit( cfruit,  slot, 0, cx, gmy+4)

        fruitleft -= 1

        cfruit = nxtfruit
        nxtfruit += 1
        if nxtfruit > #fruitinfo then nxtfruit=1 end
    end
    
end

function gen_shop_item()
    local genitem = {}

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

    -- sorter gets special map
    if (genitem.icon == 200) then
        genitem.mapx = 33
    end

    return genitem
end

function open_shop()

    while #shopitems < 3 do
        add( shopitems, gen_shop_item() )
    end

    shopytarg = 70
    shopdone = false
    shopsel=0
    buildleft = 3
end

function shop_update()
    
    local bitem = shopitems[ shopsel+1]

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
            
            if not bitem then 
                -- done button pressed
                shopytarg = 129        
                shopdone = true
            else
                -- buy/build an item   
                shopytarg = 129                     
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
            local bitem = shopitems[ shopsel+1]
            local gg = make_grid( bitem.dx, bitem.dy )            
            gg.icon = bitem.icon

            buildleft -= 1

            grid[ grid_ndx(buildx,buildy) ] = gg
            
            -- copy build tile to map
            for i=0,2 do
                for j=0,2 do
                    local gx, gy = grid_pos( i, j )
                    local mt = mget( bitem.mapx + i, 16+j )
                    mset( buildx*3 + i, 16+buildy*3+j, mt )
                end
            end    

            -- build finished
            buildx = -1
            buildy = -1
            shopytarg = 70
            
            -- replace the built item
            shopitems[shopsel+1] = gen_shop_item()

            if (buildleft==0) then
                shopytarg = 129        
                shopdone = true
                buildx = -1
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

        -- Is the game over?
        local nextr = roundinfo[round+1]
        if nextr == nil then
            -- next round
            round = round + 1
            rinfo = roundinfo[round]
            mode = 2
            fruitleft=1
            gmytarg = 30*8
        else
            if (#shopitems == 0) then 
                open_shop()
            end
            shop_update()    
        end
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
        fruitleft = 5
        shopitems = {}
        
        -- next round
        round = round + 1
        rinfo = roundinfo[round]
        mode = 2
        gmytarg = 30*8

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
    print( "upgrade " .. tostr(4-buildleft) .. "/3", 4, shopy+2, 12 )
    for ndx, item in ipairs(shopitems) do
        local xx = 8 + (ndx-1) * 35

        if (shopsel == ndx-1) then
            rectfill( xx-4, shopy+11, xx+28, shopy+55, 12 )
        end

        print( item.name, xx-3, shopy+12, 7)
        map( item.mapx, 16, xx, shopy+20, 4, 4 )
        spr( item.icon, xx+4, shopy+23, 2, 2 )        
    end

    local nextr = roundinfo[round+1]
    if (nextr != nil) then
        print( "Next:" .. nextr.desc, 10, shopy+50, 7 )
    end

    -- Done button        
    rectfill( 106, shopy+30, 124, shopy+38, (shopsel==3 and 12 or 6 ) )
    print( "done", 108, shopy+32, (shopsel==3 and 7 or 5 ) )
end

function draw_scoreinfo()
    spr( 167, cx-6, (32*8) -abs(1.0-pulse)*2  )

    local bicon = 139 +slot
    local sx, sy = (bicon % 16) * 8, flr(bicon \ 16) * 8
    circfill( 20, 36*8+8, 9, 7 )
    palt( 7, true )
    sspr( sx, sy+2, 8, 6, 13, 290, 16, 12 )
    palt( 7, false )
    
    print( buyerinfo[slot+1].name, 35, 291, 0 )
    print( buyerinfo[slot+1].name, 34, 290, 12 )

    print( buyerinfo[slot+1].desc, 40, 300, 7 )

    -- seasonal events
    if (rinfo != nil) then
        print( "rOUND " .. tostr(round) .. "/6", 46, 318, 0 )

        print( rinfo.name, 16, 329, 0 )
        print( rinfo.name, 15, 328, 12 )

        print( rinfo.desc, 18, 335, 7 )

        print( "⬆️ return to game", 30, 355, 7 )
    else
        print( "gAME oVER", 46, 318, 0 )

        local s = "final score "
        print( s, 40, 329, 0 )
        print( s, 41, 328, 10 )

        print( "--[ " .. tostr(vp) .. " ]--" , 42, 342, 7 )
        
        --print( "⬆️ return to game", 30, 355, 7 )
    end

    
end

function _draw()
    
    -- map part
    gmy = lerp( gmy, gmytarg, 0.2 )
    camera( -3, gmy )
    cls(3)
    
    palt(0,false)
    map(0,0,0,4)
    palt(0,true)

    for i=0,4 do
        for j=0,4 do
            local gx, gy = grid_pos( i, j )
            local gg = grid[ grid_ndx( i, j )]
            if (gg.icon != 0) then
                spr( gg.icon, gx-8, gy-8, 2, 2 )
            end
            --circ( gx, gy, 4, 6 )
        end
    end

    -- player sprite    
    if (fruitleft > 0 ) then 
        if (mode==1) then
            draw_dropfruit()
        elseif mode==2 then
            draw_scoreinfo()
        end
    end
    draw_fruits()
    draw_msgs()

    -- buy round    
    if #shopitems > 0 then   
        shop_draw()
    end


    camera(0,0)

    -- hud VP (coins)
    spr( 16, 100, 2 )
    local cc = tostr(vp)
    print( cc, 111, 4, 0 )
    print( cc, 110, 3, 7 )

    -- fruit left
    if fruitleft > 0 then
        local fi = fruitinfo[nxtfruit]
        local sx, sy = (fi.fnum % 16) * 8, flr(fi.fnum \ 16) * 8
        

        pal( 8, fi.cbase ) -- base color
        pal( 2, fi.cdark ) -- dark color
        pal( 14, fi.cbrite ) -- highlight color        
        sspr( sx, sy, 16, 16, 100, 12, 8, 8 )

        local fl = tostr(fruitleft)
        print( fl, 111, 14, 0 )
        print( fl, 110, 13, 7 )
        pal()
    end

    -- dbg    
    -- for i=0,4 do
    --     for j=0,4 do
    --         local gx, gy = grid_pos( i, j )
    --         circ( gx, gy, 4, 6 )
    --     end
    -- end    
    
end