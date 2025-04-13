return {
    groupName = "UnregisterConVar",
    cases = {
        {
            name = "Function exists in holytest table",
            when = HolyTest_IsModuleEnabled( "holytest" ),
            func = function()
                expect( holytest.UnregisterConVar ).to.beA( "function" )
            end
        },
        {
            name = "holytest table doesn't exist",
            when = not HolyTest_IsModuleEnabled( "holytest" ),
            func = function()
                expect( holytest ).to.beA( "nil" )
            end
        },
        {
            name = "Unregiser convars properly",
            when = HolyTest_IsModuleEnabled( "holytest" ),
            func = function()
                local convar = CreateConVar( "holytest_convartest", "1" )

                expect( convar ).to.beValid()

                holytest.UnregisterConVar( convar )

                expect( convar ).to.beInvalid()
            end
        },
    }
}
