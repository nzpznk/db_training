import './style.css';
import $ from 'jquery';
import * as mapv from 'mapv';
import 'core-js/stable';
import 'regenerator-runtime/runtime';

var carlist = [];
var cust_num_list = [];
var coorlist = [];
const RESET_ST_ID = 191487;
const RESET_ED_ID = 98028;

// 创建地图实例
var map = new BMap.Map("container");
var ggpoint = new BMap.Point(116.404, 39.915);
map.centerAndZoom(ggpoint, 16);
map.setMapStyleV2({
    styleId: "39a5fe9d5475d258799ccf92b257a2dc"
});

// 初始化地图，设置中心点坐标和地图级别
map.enableScrollWheelZoom(true);
map.addControl(new BMap.NavigationControl());
map.addControl(new BMap.ScaleControl());
var carLayer;
var pathData = []
var pathDataset = new mapv.DataSet(pathData);
var pathLayer = new mapv.baiduMapLayer(map, pathDataset, {
    strokeStyle: 'rgba(55, 50, 250, 1)',
    globalCompositeOperation: 'lighter',
    shadowColor: 'rgba(55, 50, 250, 1)',
    gradient: {0:'rgba(55, 50, 250, 1)',1:'rgba(55, 50, 250, 1)'},
    lineWidth: 5,
    draw: 'intensity'
});

function get_nodelist(node_data) {
    let posilist = [];
    let nodes = String(node_data).trim().split('\n');
    for (const node of nodes) {
        let [, logi, lati] = node.split(' ');
        posilist.push([parseFloat(logi), parseFloat(lati)]);
    }
    return posilist;
}

function get_carlist(car_data) {
    let ret = [];
    let cars = String(car_data).trim().split('\n');
    for (const x of cars) {
        let arr = x.split(' ');
        ret.push(parseInt(arr[2].split(',')[2]));
        cust_num_list.push(parseInt(arr[1]));
    }
    return ret;
}

async function init_graph() {
    await Promise.all([
        fetch('/node_file', { method: 'GET' }).then(async (node_resp) => {
            let node_dat = await node_resp.json();
            coorlist = get_nodelist(String(node_dat['node_file']));
            console.log(coorlist[0], coorlist.length);
            console.log('load nodes finished');
        }), 
        fetch('/car_file', { method: 'GET'}).then(async (car_resp) => {
            let car_dat = await car_resp.json();
            carlist = get_carlist(String(car_dat['car_file']));
            console.log(carlist[0], carlist[1], carlist[2], carlist.length);
            console.log('load car finished');
        })
    ]);
    console.log('graph initialization finished.')
}

class Controller {
    // 0:允许通过鼠标标点，或者使用radio button+输入编号标点; 
    // 1:0状态已标记起点和终点，"找车"完成后进入这个状态，可以查询车的行进路径; 
    // 2:正在进行查询，需等待
    constructor() {
        this.car_id = -1;
        this.state = 0;
        this.st_point = undefined;
        this.ed_point = undefined;
        this.st_marker = undefined;
        this.ed_marker = undefined;
        this.st_id = RESET_ST_ID;
        this.ed_id = RESET_ED_ID;
        this.pick_start = true;
        this.car_markerlist = [];
        this.pass_markerlist = [];
    }
    init() {
        this.setStartMarker(this.st_id);
        this.setDestMarker(this.ed_id);
        map.setCenter(this.st_point);
    }
    setStartMarker(node_id) {
        if (this.state == 1) return;
        $('#st_node_id')[0].value = node_id;
        $('#st_longitude').text(coorlist[node_id][0].toFixed(6));
        $('#st_latitude').text(coorlist[node_id][1].toFixed(6));
        map.removeOverlay(this.st_marker);
        this.st_point = new BMap.Point(coorlist[node_id][0], coorlist[node_id][1]);
        let marker = new BMap.Marker(this.st_point);
        this.st_id = node_id;
        this.st_marker = marker;
        map.addOverlay(this.st_marker);
        this.st_marker.setAnimation(BMAP_ANIMATION_BOUNCE);
    }
    setDestMarker(node_id) {
        if (this.state == 1) return;
        $('#ed_node_id')[0].value = node_id;
        $('#ed_longitude').text(coorlist[node_id][0].toFixed(6));
        $('#ed_latitude').text(coorlist[node_id][1].toFixed(6));
        map.removeOverlay(this.ed_marker);
        this.ed_point = new BMap.Point(coorlist[node_id][0], coorlist[node_id][1]);
        let marker = new BMap.Marker(this.ed_point);
        this.ed_id = node_id;
        this.ed_marker = marker;
        map.addOverlay(this.ed_marker);
    }
    setMarker(node_id) {
        if (this.state == 1) return;
        if (this.pick_start) {
            this.setStartMarker(node_id);
        } else {
            this.setDestMarker(node_id);
        }
    }
    getCarList() {
        if (this.state == 1) return;
        if (this.state == 2) {
            console.log('正在进行查询，请等待查询结束');
            return;
        }
        this.state = 2;
        fetch('/query_carlist', {
            method: 'POST', 
            body: JSON.stringify({
                stid: parseInt(this.st_id), 
                edid: parseInt(this.ed_id)
            }), 
            headers: new Headers({
                'Content-Type': 'application/json', 
                'Accept': 'application/json'
            })
        }).then((resp) => resp.json())
        .catch(error => {
            console.log('Error:', error);
            this.state = 1;
        }).then(val => {
            console.log(val);
            let rows = $('#car_info tr');
            for (let i = 1; i <= val.length; ++i) {
                rows.eq(i).find('.distance').text(String(val[i-1]["dist"]));
                rows.eq(i).find('.car_id').text(String(val[i-1]["car_id"]));
                rows.eq(i).find('.cust_num').text(String(val[i-1]["cust_num"]));
                rows.eq(i).find('.pre_path').prop('checked', false);
                rows.eq(i).find('.post_path').prop('checked', false);
            }
            for (let i = 0; i < val.length; ++i) {
                let n_id = carlist[val[i]["car_id"]];
                let pcar = new BMap.Point(coorlist[n_id][0], coorlist[n_id][1]);
                let marker = new BMap.Marker(pcar);
                map.addOverlay(marker);
                marker.setLabel(new BMap.Label(String(i+1), {
                    position: pcar, 
                    offset: new BMap.Size(Math.random()*30-15, -15)
                }));
                this.car_markerlist.push(marker);
            }
            console.log('marked car on the map');
            this.state = 1;
        });
    }
    getPath(car_id, isold) {
        if (this.state == 2) {
            console.log('正在进行查询，请等待查询结束');
            return;
        }
        if (this.state != 1) {
            console.log('请标记好起点和终点，并确认');
            return;
        }
        this.state = 2;
        fetch('/query_car_path', {
            method: 'POST', 
            body: JSON.stringify({
                stid: parseInt(this.st_id), 
                edid: parseInt(this.ed_id),
                carid: parseInt(car_id), 
                is_old: isold
            }), 
            headers: new Headers({
                'Content-Type': 'application/json'
            })
        }).then((resp) => resp.json())
        .catch(error => {
            console.log('Error:', error);
            this.state = 1;
        }).then(val => {
            console.log(val);
            let passlist = val['passenger'];
            let routelist = val['path'];
            this.pass_markerlist = []
            for (let n_id of passlist) {
                let ppass = new BMap.Point(coorlist[n_id][0], coorlist[n_id][1]);
                let marker = new BMap.Marker(ppass);
                map.addOverlay(marker);
                marker.setLabel(new BMap.Label(String("乘客目的地"), {
                    position: ppass, 
                    offset: new BMap.Size(0, -15)
                }));
                this.pass_markerlist.push(marker);
            }
            for (let i = 0; i < routelist.length-1; ++i) {
                let aid = routelist[i];
                let bid = routelist[i+1];
                pathData.push({
                    geometry: {
                        type: 'LineString',
                        coordinates: [[coorlist[aid][0], coorlist[aid][1]], [coorlist[bid][0], coorlist[bid][1]]]
                    }
                });
            }
            pathDataset.set(pathData);
            this.state = 1;
        })
    }
    clearPath() {
        if (this.state == 2) {
            console.log('正在进行查询，请等待查询结束');
            return;
        }
        if (this.state != 1) {
            console.log('请标记好起点和终点，并确认');
            return;
        }
        pathData = [];
        pathDataset.set(pathData);
        for(let m of this.pass_markerlist) {
            map.removeOverlay(m);
        }
        this.pass_markerlist = [];
    }
    reset() {
        if (this.state == 2) {
            console.log('正在进程查询，请等待查询结束');
            return;
        }
        if (this.st_marker) {
            map.removeOverlay(this.st_marker);
        }
        if (this.ed_marker) {
            map.removeOverlay(this.ed_marker);
        }

        controller.clearPath();

        for (let car_marker of this.car_markerlist) {
            map.removeOverlay(car_marker);
        }
        this.car_markerlist = [];
        let car_info = $('#car_info tr');
        for (let i = 1; i <= 5; ++i) {
            let row = car_info.eq(i);
            row.find('.distance').text('');
            row.find('.car_id').text('');
            row.find('.cust_num').text('');
            row.find('.pre_path').removeAttr("checked");
            row.find('.post_path').removeAttr("checked");
        }

        this.state = 0;
        this.st_id = RESET_ST_ID;
        this.ed_id = RESET_ED_ID;
        this.init();
    }
}

var controller = new Controller();

$('#st_node_id').on('change', function(e) {
    let p_id = $(this).val();
    if (p_id > 338023) {
        p_id = 338023;
    }
    if (p_id < 0) {
        p_id = 0;
    }
    console.log('st_node_id', p_id);
    controller.setStartMarker(p_id);
});

$('#ed_node_id').on('change', function(e) {
    let p_id = $(this).val();
    if (p_id > 338023) {
        p_id = 338023;
    }
    if (p_id < 0) {
        p_id = 0;
    }
    console.log('ed_node_id', p_id);
    controller.setDestMarker(p_id);
});

map.addEventListener('click', (e) => {
    let p_id = get_nearest_node(e.point);
    controller.setMarker(p_id);
});

$('#find_car_btn').on('click', function(e) {
    console.log('find car btn clicked');
    controller.getCarList();
});

$('#reset_btn').on('click', function(e) {
    console.log('reset btn clicked');
    controller.reset();
});

$('input[type=radio][name=pick_which]').change(function() {
    if ($(this).val() == 'pick_st') {
        console.log('pick start');
        controller.pick_start = true;
    } else if ($(this).val() == 'pick_ed') {
        console.log('pick destination');
        controller.pick_start = false;
    }
});

$('input[type=radio][name=which_path]').change(function() {
    let [row, is_new] = $(this).val().split(' ');
    row = parseInt(row);
    let is_old = 1 - parseInt(is_new);

    let car_id = parseInt($('#car_info tr').eq(row).find('.car_id').text());
    controller.clearPath();
    controller.getPath(car_id, is_old);
});

function get_nearest_node(point) {
    let mindist = 1e9;
    let res = 0;
    for (let i = 0; i < coorlist.length; ++i) {
        let d = Math.sqrt((point.lng-coorlist[i][0])**2 + (point.lat-coorlist[i][1])**2);
        if (d < mindist) {
            mindist = d;
            res = i;
        }
    }
    console.log(coorlist[res], res);
    return res;
}

function draw_all_cars() {
    let dataSet = new mapv.DataSet(carlist.map((val, idx) => {
        return{
            geometry: {
                type: 'Point',
                coordinates: coorlist[val]
            }, 
            count: cust_num_list[idx], 
            car_id: idx, 
            posi_id: val
        }
    }));
    let options = {
        draw: 'category',
        splitList: { 
            0: 'green',
            1: 'blue',
            2: 'orange',
            3: 'red', 
            4: 'grey'
        },
        size: 5
    }
    carLayer = new mapv.baiduMapLayer(map, dataSet, options);
    carLayer.hidden = false;
    map.addEventListener('zoomend', ()=>{
        if (!carLayer.hidden && map.getZoom() < 15) {
            console.log('hiding');
            carLayer.hidden = true;
            carLayer.hide();
        } else if (carLayer.hidden && map.getZoom() >= 15){
            console.log('showing');
            carLayer.hidden = false;
            carLayer.show();
        }
    });
    pathLayer.show();
    return dataSet;
}

(async function init() {
    await init_graph();
    draw_all_cars();
    controller.init();
})();
