/*
 * @Author: your name
 * @Date: 2020-06-11 16:09:35
 * @LastEditTime: 2020-06-16 20:27:36
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /kungfu/cli2/src/commanders/shutdown.ts
 */ 

import { killExtra, killGodDaemon, killKfc, kfKill } from '__gUtils/processUtils';
import { KF_HOME } from '__gConfig/pathConfig';



const glob = require('glob');
const fs = require('fs');
const path = require('path');

export function deleteNNFiles (rootPathName = KF_HOME) {
    return new Promise((resolve, reject) => {
        glob('**/*.nn', {
            cwd: rootPathName
        }, (err: Error, files: any) => {
            if (err) {
                reject(err)
            }
    
            files.forEach((file: any) => {
                console.log('Deleting ', path.join(rootPathName, file))
                fs.unlinkSync(path.join(rootPathName, file))
            });

            resolve(true)
        })
    })
    
}

 export const shutdown = async () => {
    try {
        console.log('Killing kfc ...')
        await killKfc();
        console.log('Killing pm2 ...')
        await killGodDaemon();
        await kfKill(['pm2']);
        console.log('Killing extra ...')
        await killExtra();
        await deleteNNFiles();
        
        console.success(`Shutdown kungfu`)
        process.exit(0)
    } catch (err) {
        console.error(err)
        process.exit(1)
    }
 }